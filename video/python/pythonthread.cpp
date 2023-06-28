
#include "pythonthread.h"

extern QQueue<cv::Mat> videoFrameQueue;
extern QMutex videoMutex;

QQueue<ROI_FRAME> roiFrameQueue;
QMutex roiMutex;

#define ERROR(str) QString("%1:%2 %3").arg(__FILE__).arg(__LINE__).arg(str)

PythonThread::PythonThread(QObject *parent) : QThread (parent)
{
    stopped = false;
    isPredict = false;
}

PythonThread::~PythonThread()
{
    close();
}

bool PythonThread::loadAlgorithmModel(const char *func_name)
{
    PyObject *ret = PyObject_CallMethod(pDetect, func_name, ""); // 加载 YoloV 模型，最耗时的过程
    if(!ret)
    {
        errorStr = ERROR("Failed to load algorithm model");
        return false;
    }
    return true;
}

bool PythonThread::predict(const char *fun, Mat srcImg, ROI_FRAME &roiFrame)
{
    PyObject *pFun = PyObject_GetAttrString(pDetect, fun); // 获取函数名
    if(!(pFun && PyCallable_Check(pFun)))
    {
        errorStr = ERROR("Failed to get detect function");
        return false;
    }
    // 将 Mat 类型 转 PyObject* 类型
    PyObject *argList = PyTuple_New(1); /* 创建只有一个元素的元组 */
    npy_intp Dims[3] = {srcImg.rows, srcImg.cols, srcImg.channels()};
    int dim = srcImg.channels() == 1 ? 1 : 3;
    PyObject *PyArray = PyArray_SimpleNewFromData(dim, Dims, NPY_UBYTE, srcImg.data); /* 创建一个数组 */
    PyTuple_SetItem(argList, 0, PyArray); /* 将数组插入元组的第一个位置 */
    // 带传参的函数执行
    PyObject *pRet = PyObject_CallObject(pFun, argList);

    _Py_XDECREF(argList); // 释放内存空间，并检测是否为null，销毁argList时同时也会销毁Pyarray
    _Py_XDECREF(pFun);    // 释放内存空间

    if(!PyTuple_Check(pRet))
    {
        errorStr = ERROR("Failed to get python return value");
        return false;// 检查返回值是否是元组类型
    }

    PyArrayObject *ret_array = nullptr;
    PyObject *calsses = nullptr;
    PyObject *confs = nullptr;

    int ret = PyArg_UnpackTuple(pRet, "ref", 3, 3, &ret_array ,&calsses, &confs); // 解析返回值
    if(!ret)
    {
        errorStr = ERROR("Failed to unpack tuple");
        _Py_XDECREF(pRet); //  PyObject_CallObject
        return false;
    }

    int size = PyList_Size(calsses); /* 获取列表长度 */
    for (int i = 0 ; i< size; i++)
    {
        PyObject *val = PyList_GetItem(calsses, i); /* 获取列表中的元素 */
        if(!val) continue;
        char *_class;
        PyArg_Parse(val, "s", &_class);             /* 解析元素 */
        roiFrame.classList.append(QString(_class));
    }

    for (int i = 0 ; i< size; i++)
    {
        PyObject *val = PyList_GetItem(confs, i);
        if(!val) continue;
        char *conf;
        PyArg_Parse(val, "s", &conf);
        roiFrame.confList.append(QString(conf));
    }

    Mat frame = Mat(ret_array->dimensions[0], ret_array->dimensions[1], CV_8UC3, PyArray_DATA(ret_array)).clone(); // 转 Mat 类型
    roiFrame.frame = frame;

    _Py_XDECREF(pRet); //  PyObject_CallObject
    return true;
}

void PythonThread::run()
{
    bool ret = init("yolov5.temp", "YoloV5"); /* python算法初始化，模块为temp.py,类名为YoloV5 */
    if(!ret)
    {
        qDebug() << errorString();
        return;
    }

    ret = loadAlgorithmModel("load_model");
    if(!ret)
    {
        qDebug() << errorString();
        return;
    }

    Mat srcFrame;
    ROI_FRAME dstFrame;
    while (stopped)
    {
        if(isPredict)
        {
            videoMutex.lock();
            if(!videoFrameQueue.isEmpty())
            {
                srcFrame = videoFrameQueue.dequeue();
                if(videoFrameQueue.size() > 3) videoFrameQueue.clear();
//                qDebug() << "opencv: " + QString::number(videoFrameQueue.size());
            }
            videoMutex.unlock();

            ROI_FRAME dstFrame;
            ret = predict("detect", srcFrame, dstFrame); /* 调用类中的检测函数derect */
            if(!ret) continue;

            roiMutex.lock();
            roiFrameQueue.enqueue(dstFrame);
            roiMutex.unlock();
        }
        usleep(1);
    }
}

QString PythonThread::errorString()
{
    return errorStr;
}

bool PythonThread::init(const char *module_name, const char *class_name)
{
    Py_SetPythonHome(reinterpret_cast<const wchar_t*>(L"G:\\RJAZ\\Miniconnda\\data")); // 设置 python 环境目录
    Py_Initialize(); // 初始化
    if(!Py_IsInitialized())
    {
        errorStr = ERROR("python initialization failed.");
        return false;
    }
    PyRun_SimpleString("import sys");                             // 加载 sys 模块
    PyRun_SimpleString("sys.path.append('/')");                   // 设置 python 文件搜索路径
    PyRun_SimpleString("sys.path.append('./yolov5')");            // 将算法添加进python搜索路径

    PyObject *pModule = PyImport_ImportModule(module_name);       // 调用的文件名
     if(!pModule)
     {
         Py_Finalize();
         errorStr = ERROR("Failed to import python module.");
         return false;
     }

     PyObject *pDict = PyModule_GetDict(pModule);                  // 加载文件中的函数名、类名
     _Py_XDECREF(pModule);
     if(!pDict)
     {
         Py_Finalize();
         errorStr = ERROR("Failed to get the module dictionary.");
         return false;
     }

     PyObject *pClass = PyDict_GetItemString(pDict, class_name);   // 获取类名
     _Py_XDECREF(pDict);
     if(!pClass)
     {
         Py_Finalize();
         errorStr = ERROR("Failed to get class name.");
         return false;
     }

     pDetect = PyObject_CallObject(pClass, nullptr);               // 实例化对象，相当于调用'__init__(self)',参数为null
     _Py_XDECREF(pClass);
     if(!pDetect)
     {
         Py_Finalize();
         errorStr = ERROR("Failed to instantiate the python class.");
         return false;
     }

     if (_import_array() < 0)
     {
         Py_Finalize();
         errorStr = ERROR("Failed to import numpy");
         return false;                        // 加载 numpy 模块
     }
     return true;
}

void PythonThread::deinit()
{
    _Py_XDECREF(pDetect);
    Py_Finalize();
}

void PythonThread::open()
{
    QMutexLocker locker(&mutex);
    stopped = true;
    isPredict = true;
}

void PythonThread::close()
{
    QMutexLocker locker(&mutex);
    stopped = false;
}

void PythonThread::pause()
{
    QMutexLocker locker(&mutex);
    isPredict = false;
}

void PythonThread::next()
{
    QMutexLocker locker(&mutex);
    isPredict = true;
}
