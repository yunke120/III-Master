
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

bool PythonThread::PyObject_ToStringList(PyObject* obj, std::vector<std::string>& outList) {
    if (!PyList_Check(obj)) {
        return false;
    }

    Py_ssize_t size = PyList_Size(obj);
    outList.reserve(size);

    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject* item = PyList_GetItem(obj, i);
        if (!item || !PyUnicode_Check(item)) {
            return false;
        }
        PyObject* utf8StrObj = PyUnicode_AsUTF8String(item);
        if (!utf8StrObj) {
            return false;
        }
        const char* utf8Str = PyBytes_AsString(utf8StrObj);
        if (!utf8Str) {
            Py_DECREF(utf8StrObj);
            return false;
        }
        outList.emplace_back(utf8Str);
        Py_DECREF(utf8StrObj);
    }

    return true;
}

// 辅助函数: 将 PyObject 转换为 QString 列表
bool PythonThread::PyObject_ToStringList(PyObject* obj, QStringList& outList) {
    if (!PyList_Check(obj)) {
        return false;
    }

    Py_ssize_t size = PyList_Size(obj);
    outList.reserve(size);

    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject* item = PyList_GetItem(obj, i);
        if (!item || !PyUnicode_Check(item)) {
            return false;
        }
        PyObject* utf8StrObj = PyUnicode_AsUTF8String(item);
        if (!utf8StrObj) {
            return false;
        }
        const char* utf8Str = PyBytes_AsString(utf8StrObj);
        if (!utf8Str) {
            Py_DECREF(utf8StrObj);
            return false;
        }
        outList.append(QString::fromUtf8(utf8Str));
        Py_DECREF(utf8StrObj);
    }

    return true;
}

// 辅助函数: 将 PyObject 转换为 UTF-8 编码的字符串
const char* PythonThread::PyUnicode_AsUTF8(PyObject* obj) {
    PyObject* utf8StrObj = PyUnicode_AsUTF8String(obj);
    if (!utf8StrObj) {
        return nullptr;
    }

    const char* utf8Str = PyBytes_AsString(utf8StrObj);
    Py_DECREF(utf8StrObj);

    return utf8Str;
}

bool PythonThread::predict(const char *fun, Mat srcImg, ROI_FRAME &roiFrame)
{
    PyObject* pFun = PyObject_GetAttrString(pDetect, fun);
    if (!(pFun && PyCallable_Check(pFun))) {
        errorStr = ERROR("Failed to get detect function");
        PyErr_Clear(); // 清除异常状态
        Py_XDECREF(pFun);
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

    // 获取类别列表
    if (!PyObject_ToStringList(calsses, roiFrame.classList)) {
        errorStr = ERROR("Failed to parse classes list");
        PyErr_Clear();
        Py_XDECREF(pRet);
        return false;
    }

    // 获取置信度列表
    if (!PyObject_ToStringList(confs, roiFrame.confList)) {
        errorStr = ERROR("Failed to parse confidences list");
        PyErr_Clear();
        Py_XDECREF(pRet);
        return false;
    }

    Mat frame = Mat(ret_array->dimensions[0], ret_array->dimensions[1], CV_8UC3, PyArray_DATA(ret_array)).clone(); // 转 Mat 类型
    roiFrame.frame = frame;

    Py_XDECREF(pRet);

    return true;
}

void PythonThread::run()
{
    bool ret = init(L"G:\\RJAZ\\Miniconnda\\data", "yolov5.temp", "YoloV5"); /* python算法初始化，模块为temp.py,类名为YoloV5 */
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
                /*if(videoFrameQueue.size() > 3)*/ videoFrameQueue.clear();
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
    deinit();
}

QString PythonThread::errorString()
{
    return errorStr;
}

bool PythonThread::initializePythonInterpreter(const std::wstring& pythonHome) {
    Py_SetPythonHome(pythonHome.c_str()); // 设置 python 环境目录
    Py_Initialize(); // 初始化

    if (Py_IsInitialized() == 0) {
        std::cerr << "Python initialization failed." << std::endl;
        return false;
    }

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('/')");                 // 设置 python 文件搜索路径
    PyRun_SimpleString("sys.path.append('./yolov5')");          // 将算法添加进 python 搜索路径

    return true;
}

bool PythonThread::importPythonModule(const std::string& moduleName, const std::string& className,
                        PyObject*& pModule, PyObject*& pClass) {
    pModule = PyImport_ImportModule(moduleName.c_str());         // 调用的文件名
    if (!pModule) {
        std::cerr << "Failed to import python module." << std::endl;
        return false;
    }

    PyObject *pDict = PyModule_GetDict(pModule);                // 加载文件中的函数名、类名
    if (!pDict) {
        std::cerr << "Failed to get the module dictionary." << std::endl;
        Py_XDECREF(pModule);
        return false;
    }

    pClass = PyDict_GetItemString(pDict, className.c_str());    // 获取类名
    if (!pClass) {
        std::cerr << "Failed to get class name." << std::endl;
        Py_XDECREF(pModule);
        Py_XDECREF(pDict);
        return false;
    }

    return true;
}

bool PythonThread::instantiatePythonClass(PyObject* pClass, PyObject*& pDetect) {
    pDetect = PyObject_CallObject(pClass, nullptr);             // 实例化对象，相当于调用'__init__(self)',参数为null
    if (!pDetect) {
        std::cerr << "Failed to instantiate the python class." << std::endl;
        Py_XDECREF(pClass);
        return false;
    }

    return true;
}

bool PythonThread::importNumPy() {
    if (_import_array() < 0) {
        std::cerr << "Failed to import numpy." << std::endl;
        return false;                      // 加载 numpy 模块
    }

    return true;
}

bool PythonThread::init(const std::wstring& pythonHome, const std::string& moduleName,
                        const std::string& className) {
    if (!initializePythonInterpreter(pythonHome)) {
        errorStr = ERROR("Python initialization failed.");
        return false;
    }

    PyObject* pModule = nullptr;
    PyObject* pClass = nullptr;
    if (!importPythonModule(moduleName, className, pModule, pClass)) {
        Py_Finalize();
        errorStr = ERROR("Failed to import python module.");
        return false;
    }

    pDetect = nullptr;
    if (!instantiatePythonClass(pClass, pDetect)) {
        Py_XDECREF(pModule);
        Py_Finalize();
        errorStr = ERROR("Failed to instantiate the python class.");
        return false;
    }

    if (!importNumPy()) {
        Py_XDECREF(pDetect);
        Py_XDECREF(pClass);
        Py_XDECREF(pModule);
        Py_Finalize();
        errorStr = ERROR("Failed to import numpy.");
        return false;
    }

    return true;
}

bool PythonThread::loadAlgorithmModel(const char *func_name) {
    errorStr = ""; // 重置错误信息

    PyObject* ret = PyObject_CallMethod(pDetect, func_name, ""); // 加载 YoloV 模型，最耗时的过程
    if (!ret) {
        errorStr = ERROR("Failed to load algorithm model");
        return false;
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
