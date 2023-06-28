#ifndef OPENCVTHREAD_H
#define OPENCVTHREAD_H

#include <QThread>
#include <QImage>
#include <QMutex>
#include <QQueue>

#include <imgproc/imgproc.hpp>
#include <core.hpp>
#include <videoio.hpp>
#include <highgui.hpp>

using namespace cv;
using namespace std;


class OpencvThread : public QThread
{
    Q_OBJECT
public:
    explicit OpencvThread(QObject *parent = nullptr);
protected:
    void run();

private:
    volatile bool stopped;          //线程停止标志位
    VideoCapture *pCap = nullptr;
    QMutex mutex;
    QString Url;
    int Addr;
    bool addrType;
public slots:
    //设置视频流地址
    void setAddr(const QString url);
    void setAddr(int index);
    void open();
    void close();

signals:
    void sigReset(void);
};


#endif /* OPENCVTHREAD_H */
