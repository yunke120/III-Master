﻿#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "video/widget/videowidget.h"

class QAbstractButton;

namespace Ui {
class frmMain;
}

class frmMain : public QWidget
{
    Q_OBJECT

public:
    explicit frmMain(QWidget *parent = 0);
    ~frmMain();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::frmMain *ui;

    QList<int> iconsMain;
    QList<QAbstractButton *> btnsMain;

    QList<int> iconsConfig;
    QList<QAbstractButton *> btnsConfig;

    QSerialPort *pSerial;                       /* 串口对象 */

private:
    //根据QSS样式获取对应颜色值
    QString borderColor;
    QString normalBgColor;
    QString darkBgColor;
    QString normalTextColor;
    QString darkTextColor;

    void getQssColor(const QString &qss, const QString &flag, QString &color);
    void getQssColor(const QString &qss, QString &textColor,
                     QString &panelColor, QString &borderColor,
                     QString &normalColorStart, QString &normalColorEnd,
                     QString &darkColorStart, QString &darkColorEnd,
                     QString &highColor);

private slots:
    void initForm();
    void initStyle();
    void buttonClick();
    void initLeftMain();
    void initLeftConfig();
    void leftMainClick();
    void leftConfigClick();
    void initSerialPort();

private slots:
    void on_btnMenu_Min_clicked();
    void on_btnMenu_Max_clicked();
    void on_btnMenu_Close_clicked();
    void on_btnOpenVideo_clicked();

    void slotSerialReadyRead();
    void on_btnRobotUp_clicked();
    void on_btnRobotDown_clicked();
    void on_btnRobotLeft_clicked();
    void on_btnRobotRight_clicked();
    void on_btnRobotStop_clicked();
};

#endif // FRMMAIN_H
