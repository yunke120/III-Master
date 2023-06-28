#ifndef PYTHONTHREAD_H
#define PYTHONTHREAD_H


#include <QThread>
#include <QImage>
#include <QMutex>
#include <QQueue>
#include <QDebug>
#include <QTimer>

#include <string>
#include <iostream>
#include <vector>

extern "C"
{
#include <python/Python.h>
#include <numpy/ndarrayobject.h>
}


#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

typedef struct {
    cv::Mat frame;        /* 带有检测框的图像 */
    QStringList classList;
    QStringList confList;
}ROI_FRAME;

using namespace cv;
class PythonThread : public QThread
{
    Q_OBJECT
public:
    explicit PythonThread(QObject *parent = nullptr);
    ~PythonThread();

private:
    bool initializePythonInterpreter(const std::wstring& pythonHome);
    bool importPythonModule(const std::string& moduleName, const std::string& className,
                            PyObject*& pModule, PyObject*& pClass);
    bool instantiatePythonClass(PyObject* pClass, PyObject*& pDetect);
    bool importNumPy();
    bool init(const std::wstring& pythonHome, const std::string& moduleName,
                            const std::string& className);
    bool PyObject_ToStringList(PyObject* obj, std::vector<std::string>& outList);
    bool PyObject_ToStringList(PyObject* obj, QStringList& outList);
    const char* PyUnicode_AsUTF8(PyObject* obj);
    void deinit();
    bool loadAlgorithmModel(const char *func_name);
    bool predict(const char *fun, Mat srcImg, ROI_FRAME &roiFrame);

protected:
    void run(void);

public slots:
    QString errorString();

    void open();
    void close();
    void pause();
    void next();
private:
    QString errorStr;
    PyObject *pDetect = nullptr;   /* python检测图像类 */

    QMutex mutex;
    bool stopped;
    bool isPredict;
};


#endif /* PYTHONTHREAD_H */
