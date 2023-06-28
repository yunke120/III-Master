#ifndef PYTHONTHREAD_H
#define PYTHONTHREAD_H


#include <QThread>
#include <QImage>
#include <QMutex>
#include <QQueue>
#include <QDebug>
#include <QTimer>

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
    bool init(const char *module_name, const char *class_name);
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
