
#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
#include <QtWidgets>
#endif

#include <QTimer>

#include "video/opencv2/opencvthread.h"

#ifdef USING_PYTHON_THREAD
#include "video/python/pythonthread.h"
#endif

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();

    QImage cvMat2QImage(const Mat &mat);
protected:
    void paintEvent(QPaintEvent *);

private:
    OpencvThread *thread1;
    QTimer *timer;
    QImage image;
#ifdef USING_PYTHON_THREAD
    PythonThread *thread2;
#endif
private slots:
    void slotGetImage(void);

public slots:
    void setAddr(const QString Url);
    void setAddr(int index);

    void open();
    void clear();
    void close();
};



#endif /* VIDEOWIDGET_H */
