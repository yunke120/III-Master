#include "videowidget.h"

extern QQueue<cv::Mat> videoFrameQueue;
extern QMutex videoMutex;

extern QQueue<ROI_FRAME> roiFrameQueue;
extern QMutex roiMutex;

VideoWidget::VideoWidget(QWidget *parent) : QWidget (parent)
{
    thread1 = new OpencvThread(this);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &VideoWidget::slotGetImage);

    thread2 = new PythonThread(this);

}

VideoWidget::~VideoWidget()
{
    close();
}

void VideoWidget::paintEvent(QPaintEvent *)
{
    if (image.isNull()) {
        return;
    }

    QPainter painter(this);
#if 0
    //image = image.scaled(this->size(), Qt::KeepAspectRatio);
    //按照比例自动居中绘制
    int pixX = rect().center().x() - image.width() / 2;
    int pixY = rect().center().y() - image.height() / 2;
    QPoint point(pixX, pixY);
    painter.drawImage(point, image);
#else
    painter.drawImage(this->rect(), image);
#endif
}

void VideoWidget::slotGetImage()
{
    Mat srcFrame;
#if 0
    videoMutex.lock();
    if(!videoFrameQueue.isEmpty())
        srcFrame = videoFrameQueue.dequeue();
    videoMutex.unlock();
#else
    ROI_FRAME roiFrame;
    roiMutex.lock();
    if(!roiFrameQueue.isEmpty())
    {
        roiFrame = roiFrameQueue.dequeue();
        if(roiFrameQueue.size() > 3) roiFrameQueue.clear();
//        qDebug() << "python: " + QString::number(roiFrameQueue.size());
    }

    roiMutex.unlock();
    srcFrame = roiFrame.frame;
#endif

    image = cvMat2QImage(roiFrame.frame);

    update();
}

void VideoWidget::setAddr(const QString Url)
{
    thread1->setAddr(Url);
}

void VideoWidget::setAddr(int index)
{
    thread1->setAddr(index);
}

void VideoWidget::open()
{
    thread1->open();
    thread1->start();
    timer->start(50);

    thread2->open();
    thread2->start();
}

void VideoWidget::clear()
{
    image = QImage();
    update();
}

void VideoWidget::close()
{
    if(timer->isActive())
    {
        timer->stop();
    }

    if(thread1->isRunning())
    {
        videoMutex.lock();
        videoFrameQueue.clear();
        videoMutex.unlock();

        thread1->close();
        thread1->quit();
        thread1->wait(500);
    }

    if(thread2->isRunning())
    {
        roiMutex.lock();
        roiFrameQueue.clear();
        roiMutex.unlock();

        thread2->close();
        thread2->quit();
        thread2->wait(500);
    }
    QTimer::singleShot(1, this, SLOT(clear()));
}



QImage VideoWidget::cvMat2QImage(const Mat &mat)
{
    if (mat.type() == CV_8UC3)               // 8-bits unsigned, NO. OF CHANNELS = 3
    {
        const uchar *pSrc = reinterpret_cast<const uchar*>(mat.data);  // Copy input Mat
        QImage image(pSrc, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_RGB888); // Create QImage with same dimensions as input Mat
        return image.rgbSwapped();
    }
//    else if (mat.type() == CV_8UC1)                                      // 8-bits unsigned, NO. OF CHANNELS = 1
//    {
//        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
//        image.setColorCount(256);                                  // Set the color table (used to translate colour indexes to qRgb values)
//        for (int i = 0; i < 256; i++)
//            image.setColor(i, qRgb(i, i, i));

//        uchar *pSrc = mat.data;                                    // Copy input Mat
//        for (int row = 0; row < mat.rows; row++)
//        {
//            uchar *pDest = image.scanLine(row);
//            memcpy(pDest, pSrc, static_cast<size_t>(mat.cols));
//            pSrc += mat.step;
//        }
//        return image;
//    }
//    else if (mat.type() == CV_8UC4)
//    {
//        const uchar *pSrc = reinterpret_cast<const uchar*>(mat.data);
//        QImage image(pSrc, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_ARGB32);
//        return image.copy();
//    }
    else
    {
        return QImage();  // Mat could not be converted to QImage
    }
}
