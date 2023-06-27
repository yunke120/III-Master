
#include "OpencvThread.h"

QQueue<cv::Mat> videoFrameQueue;
QMutex videoMutex;

OpencvThread::OpencvThread(QObject *parent)
{
    Q_UNUSED(parent)
    setObjectName("OpencvThread");
    stopped = false;
}

void OpencvThread::run()
{
    if(addrType)
        pCap = new VideoCapture(Url.toStdString());
    else
        pCap = new VideoCapture(Addr);

    if(pCap == nullptr || !pCap->isOpened())
    {
        emit sigReset();
        return;
    }

    Mat frame;
    int timeout = 3;

    while(stopped)
    {
        *pCap >> frame;
        if(frame.empty())
        {
            timeout --;
            if(timeout == 0)
                break;
            continue;
        }
        else
        {
            timeout = 3;
            videoMutex.lock();
            videoFrameQueue.enqueue(frame);
            videoMutex.unlock();
        }

        msleep(10);
    }
    if(timeout == 0)
    {
        emit sigReset();
    }
    stopped = false;
    pCap->release();
    delete pCap;
    pCap = nullptr;
    return;
}

void OpencvThread::setAddr(const QString url)
{
    this->Url = url;
    addrType = true;
}

void OpencvThread::setAddr(int index)
{
    this->Addr = index;
    addrType = false;
}

void OpencvThread::play()
{
    QMutexLocker locker(&mutex);
    stopped = true;
}

void OpencvThread::stop()
{
    QMutexLocker locker(&mutex);
    stopped = false;
}
