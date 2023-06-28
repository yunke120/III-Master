#include "videowidget.h"

extern QQueue<cv::Mat> videoFrameQueue;
extern QMutex videoMutex;


VideoWidget::VideoWidget(QWidget *parent) : QWidget (parent)
{
    thread = new OpencvThread(this);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &VideoWidget::slotGetImage);
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
    videoMutex.lock();
    if(!videoFrameQueue.isEmpty())
        srcFrame = videoFrameQueue.dequeue();
    videoMutex.unlock();

    image = cvMat2QImage(srcFrame);

    update();
}

void VideoWidget::setAddr(const QString Url)
{
    thread->setAddr(Url);
}

void VideoWidget::setAddr(int index)
{
    thread->setAddr(index);
}

void VideoWidget::open()
{
    thread->play();
    thread->start();
    timer->start(50);
}

void VideoWidget::clear()
{
    image = QImage();
    update();
}

void VideoWidget::close()
{
    if(thread->isRunning())
    {
        videoMutex.lock();
        videoFrameQueue.clear();
        videoMutex.unlock();

        thread->stop();
        thread->quit();
        thread->wait(500);
        timer->stop();
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
