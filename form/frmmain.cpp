#pragma execution_character_set("utf-8")

#include "frmmain.h"
#include "ui_frmmain.h"
#include "iconhelper.h"
#include "quihelper.h"
#include "log4qt/log.h"

frmMain::frmMain(QWidget *parent) : QWidget(parent), ui(new Ui::frmMain)
{
    // 837464779 2:10
    ui->setupUi(this);
    this->initForm();
    this->initStyle();
    this->initLeftMain();
    this->initLeftConfig();
    this->initSerialPort();
    this->initLogSql();
    connect(ui->stackedWidget3, &QStackedWidget::currentChanged, this, &frmMain::slotConfigChange);

    pStatusWidget = new StatusWidget(this,StatusWidget::EnterDirection::LeftIn, StatusWidget::LeaveDirection::BottomOut);
    pStatusWidget->setStyleSheet("QWidget{border-radius:10px}");
    pStatusWidget->hide();
}

frmMain::~frmMain()
{
    delete ui;
}

/**
 * @brief 判断文件夹是否存在，不存在就创建
 *
 * @param folder 文件夹
 * @return true 存在或创建成功
 * @return false 不存在或创建失败
 */
bool frmMain::createFolder(const QString &folder)
{
    QDir _dir(folder);
    if(_dir.exists()) return true;
    else return _dir.mkdir(folder);
}

bool frmMain::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->widgetTitle) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            on_btnMenu_Max_clicked();
        }
    }
    return QWidget::eventFilter(watched, event);
}

void frmMain::getQssColor(const QString &qss, const QString &flag, QString &color)
{
    int index = qss.indexOf(flag);
    if (index >= 0) {
        color = qss.mid(index + flag.length(), 7);
    }
    //qDebug() << TIMEMS << flag << color;
}

void frmMain::getQssColor(const QString &qss, QString &textColor, QString &panelColor,
                          QString &borderColor, QString &normalColorStart, QString &normalColorEnd,
                          QString &darkColorStart, QString &darkColorEnd, QString &highColor)
{
    getQssColor(qss, "TextColor:", textColor);
    getQssColor(qss, "PanelColor:", panelColor);
    getQssColor(qss, "BorderColor:", borderColor);
    getQssColor(qss, "NormalColorStart:", normalColorStart);
    getQssColor(qss, "NormalColorEnd:", normalColorEnd);
    getQssColor(qss, "DarkColorStart:", darkColorStart);
    getQssColor(qss, "DarkColorEnd:", darkColorEnd);
    getQssColor(qss, "HighColor:", highColor);
}

void frmMain::appendDatat2LogWidget(const QList<QVariantMap> &data)
{
    ui->logTableWidget->setRowCount(0);
    int numRows = ui->logTableWidget->rowCount();
    ui->logTableWidget->setRowCount(numRows + data.size());

    for (const QVariantMap& log : data) {
        QTableWidgetItem *item_0 = new QTableWidgetItem(QString::number(numRows+1));
        item_0->setTextAlignment(Qt::AlignCenter); // 居中对齐
        ui->logTableWidget->setItem(numRows, 0, item_0);

        QTableWidgetItem *item_1 = new QTableWidgetItem(log["timestamp"].toDateTime().toString());
        item_1->setTextAlignment(Qt::AlignCenter); // 居中对齐
        ui->logTableWidget->setItem(numRows, 1, item_1);

        QTableWidgetItem *item_2 = new QTableWidgetItem(log["level"].toString());
        item_2->setTextAlignment(Qt::AlignCenter); // 居中对齐
        ui->logTableWidget->setItem(numRows, 2, item_2);

        QTableWidgetItem *item_3 = new QTableWidgetItem(log["message"].toString());
//        item_3->setTextAlignment(Qt::AlignCenter); // 居中对齐
        ui->logTableWidget->setItem(numRows, 3, item_3);
        numRows++;
    }
}

uint8_t frmMain::checkNumber(uint8_t *data, unsigned char len, unsigned char mode)
{

    unsigned char check_sum=0,k;

    for(k=0;k<len;k++)
    {
        check_sum=check_sum^data[k];
    }
    return check_sum;

}

void frmMain::initForm()
{
    //设置无边框
    QUIHelper::setFramelessForm(this);
    //设置图标
    IconHelper::setIcon(ui->labIco, 0xf073, 30);
    IconHelper::setIcon(ui->btnMenu_Min, 0xf068);
    IconHelper::setIcon(ui->btnMenu_Max, 0xf067);
    IconHelper::setIcon(ui->btnMenu_Close, 0xf00d);

    //ui->widgetMenu->setVisible(false);
    ui->widgetTitle->setProperty("form", "title");
    //关联事件过滤器用于双击放大
    ui->widgetTitle->installEventFilter(this);
    ui->widgetTop->setProperty("nav", "top");

    QFont font;
    font.setPixelSize(25);
    ui->labTitle->setFont(font);
    ui->labTitle->setText("III-Robot 控制终端");
    this->setWindowTitle(ui->labTitle->text());

    ui->stackedWidget->setStyleSheet("QLabel{font:16px;} QPushButton{font:16px;}");

    QSize icoSize(32, 32);
    int icoWidth = 85;

    //设置顶部导航按钮
    QList<QAbstractButton *> tbtns = ui->widgetTop->findChildren<QAbstractButton *>();
    foreach (QAbstractButton *btn, tbtns) {
        btn->setIconSize(icoSize);
        btn->setMinimumWidth(icoWidth);
        btn->setCheckable(true);
        connect(btn, SIGNAL(clicked()), this, SLOT(buttonClick()));
    }

    ui->btnMain->click();

    ui->widgetLeftMain->setProperty("flag", "left");
    ui->widgetLeftConfig->setProperty("flag", "left");
    ui->page1->setStyleSheet(QString("QWidget[flag=\"left\"] QAbstractButton{min-height:%1px;max-height:%1px;}").arg(60));
    ui->page2->setStyleSheet(QString("QWidget[flag=\"left\"] QAbstractButton{min-height:%1px;max-height:%1px;}").arg(25));
}

void frmMain::initStyle()
{
    //加载样式表
    QString qss;
    QFile file(":/qss/blacksoft.css");
    if (file.open(QFile::ReadOnly)) {
        qss = QLatin1String(file.readAll());
        QString paletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(paletteColor));
        qApp->setStyleSheet(qss);
        file.close();
    }

    //先从样式表中取出对应的颜色
    QString textColor, panelColor, borderColor, normalColorStart, normalColorEnd, darkColorStart, darkColorEnd, highColor;
    getQssColor(qss, textColor, panelColor, borderColor, normalColorStart, normalColorEnd, darkColorStart, darkColorEnd, highColor);

    //将对应颜色设置到控件
    this->borderColor = highColor;
    this->normalBgColor = normalColorStart;
    this->darkBgColor = panelColor;
    this->normalTextColor = textColor;
    this->darkTextColor = normalTextColor;

}

void frmMain::buttonClick()
{
    QAbstractButton *b = (QAbstractButton *)sender();
    QString name = b->text();

    QList<QAbstractButton *> tbtns = ui->widgetTop->findChildren<QAbstractButton *>();
    foreach (QAbstractButton *btn, tbtns) {
        btn->setChecked(btn == b);
    }
    qDebug() << name;
    if (name == "主界面") {
        ui->stackedWidget->setCurrentIndex(0);
    } else if (name == "系统设置") {
        ui->stackedWidget->setCurrentIndex(1);
    } else if (name == "日志查询") {
        ui->stackedWidget->setCurrentIndex(2);
    } else if (name == "调试帮助") {
        ui->stackedWidget->setCurrentIndex(3);
    } else if (name == "用户退出") {
        exit(0);
    }
}

void frmMain::initLeftMain()
{
    iconsMain << 0xf030 << 0xf03e << 0xf247;
    btnsMain << ui->tbtnMain1 << ui->tbtnMain2 << ui->tbtnMain3;

    int count = btnsMain.count();
    for (int i = 0; i < count; ++i) {
        QToolButton *btn = (QToolButton *)btnsMain.at(i);
        btn->setCheckable(true);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        connect(btn, SIGNAL(clicked(bool)), this, SLOT(leftMainClick()));
    }

    IconHelper::StyleColor styleColor;
    styleColor.position = "left";
    styleColor.iconSize = 18;
    styleColor.iconWidth = 35;
    styleColor.iconHeight = 25;
    styleColor.borderWidth = 4;
    styleColor.borderColor = borderColor;
    styleColor.setColor(normalBgColor, normalTextColor, darkBgColor, darkTextColor);
    IconHelper::setStyle(ui->widgetLeftMain, btnsMain, iconsMain, styleColor);
    ui->tbtnMain1->click();
}

void frmMain::initLeftConfig()
{
    iconsConfig << 0xf031 << 0xf036 << 0xf249 << 0xf055 << 0xf05a << 0xf249;
    btnsConfig << ui->tbtnConfig1 << ui->tbtnConfig2 << ui->tbtnConfig3 << ui->tbtnConfig5 << ui->tbtnConfig6;

    int count = btnsConfig.count();
    for (int i = 0; i < count; ++i) {
        QToolButton *btn = (QToolButton *)btnsConfig.at(i);
        btn->setCheckable(true);
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        connect(btn, SIGNAL(clicked(bool)), this, SLOT(leftConfigClick()));
    }

    IconHelper::StyleColor styleColor;
    styleColor.position = "left";
    styleColor.iconSize = 16;
    styleColor.iconWidth = 20;
    styleColor.iconHeight = 20;
    styleColor.borderWidth = 3;
    styleColor.borderColor = borderColor;
    styleColor.setColor(normalBgColor, normalTextColor, darkBgColor, darkTextColor);
    IconHelper::setStyle(ui->widgetLeftConfig, btnsConfig, iconsConfig, styleColor);
    ui->tbtnConfig1->click();
}

void frmMain::leftMainClick()
{
    QAbstractButton *b = (QAbstractButton *)sender();
    QString name = b->text();

    int count = btnsMain.count();
    for (int i = 0; i < count; ++i) {
        QAbstractButton *btn = btnsMain.at(i);
        btn->setChecked(btn == b);
    }

    if (name == "视频监控") {
        ui->stackedWidget2->setCurrentIndex(0);
    } else if (name == "地图监控") {
        ui->stackedWidget2->setCurrentIndex(1);
    } else if (name == "设备监控") {
        ui->stackedWidget2->setCurrentIndex(2);
    }
}
void frmMain::leftConfigClick()
{
    QToolButton *b = (QToolButton *)sender();
    QString name = b->text();

    int count = btnsConfig.count();
    for (int i = 0; i < count; ++i) {
        QAbstractButton *btn = btnsConfig.at(i);
        btn->setChecked(btn == b);
        if(btn == b)
            ui->stackedWidget3->setCurrentIndex(i);
    }

}

void frmMain::initSerialPort()
{
    pSerial = new QSerialPort(this);
    pSerial->setBaudRate(QSerialPort::Baud115200);
    pSerial->setDataBits(QSerialPort::Data8);
    pSerial->setStopBits(QSerialPort::OneStop);
    pSerial->setParity(QSerialPort::NoParity);
    pSerial->setFlowControl(QSerialPort::NoFlowControl);
    connect(pSerial, &QSerialPort::readyRead, this, &frmMain::slotSerialReadyRead);

    ui->cbPorts->clear();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) /* 初始化串口列表 */
    {
        ui->cbPorts->addItem(info.portName());
    }
}

void frmMain::initConfig()
{
    QSettings settings(CONFIG_FILEPATH, QSettings::IniFormat);

}

void frmMain::initLogSql()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString currentDate = currentDateTime.toString("yyyy-MM-dd");

    // 检查是否需要生成新的数据库文件
    static QString currentDbName;
    static QString logsDirectory = "logs";
    if (currentDate != currentDbName) {
        currentDbName = logsDirectory + "/" + currentDate + ".db";
    }

    plogSql = new LoggerSql(currentDbName);

    // 记录一些日志
//    plogSql->log(LoggerSql::eINFO, "This is an informational message.");
//    plogSql->log(LoggerSql::eERROR, "An error occurred.");
//    plogSql->log(LoggerSql::eINFO, "Another informational message.");
//    plogSql->log(LoggerSql::eWARN, "中文测试1234567890");

    // 获取并打印所有日志
//    QList<QVariantMap> allLogs = plogSql->getLogs();
//    qDebug() << "All Logs:";
//    for (const QVariantMap& log : allLogs) {
//        qDebug() << "ID: " << log["id"].toInt()
//                 << "Timestamp: " << log["timestamp"].toDateTime().toString()
//                 << "Level: " << log["level"].toString()
//                 << "Message: " << log["message"].toString();
//    }

//    // 获取并打印特定等级的日志
//    QList<QVariantMap> errorLogs = plogSql->getLogs(LoggerSql::LogLevel::eWARN);
//    qDebug() << errorLogs.length();
//    qDebug() << "Error Logs:";
//    for (const QVariantMap& log : errorLogs) {
//        qDebug() << "ID: " << log["id"].toInt()
//                 << "Timestamp: " << log["timestamp"].toDateTime().toString()
//                 << "Level: " << log["level"].toString()
//                 << "Message: " << log["message"].toString();
//    }

    ui->logTableWidget->verticalHeader()->setVisible(false);
    ui->logTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 调整表格列宽

    // 设置表头
    QStringList headers;
    headers << "条目" << "时间" << "日志等级" << "消息";
    ui->logTableWidget->setColumnCount(4);
    ui->logTableWidget->setHorizontalHeaderLabels(headers);
    // 设置列的stretch属性，让它们自动拉伸
    ui->logTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->logTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->logTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    ui->logTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
}

void frmMain::on_btnMenu_Min_clicked()
{
    showMinimized();
}

void frmMain::on_btnMenu_Max_clicked()
{
    static bool max = false;
    static QRect location = this->geometry();

    if (max) {
        this->setGeometry(location);
    } else {
        location = this->geometry();
        this->setGeometry(QUIHelper::getScreenRect());
    }

    this->setProperty("canMove", max);
    max = !max;
}

void frmMain::on_btnMenu_Close_clicked()
{
    close();
}


void frmMain::on_btnOpenVideo_clicked()
{
    if(ui->btnOpenVideo->text() == "打开视频")
    {
        ui->btnOpenVideo->setText("关闭视频");
//        ui->widgetVideo->setAddr("rtsp://127.0.0.1:8554/main"); // 网络流
        ui->widgetVideo->setAddr("./dream.mp4"); // 本地视频文件
//        ui->widgetVideo->setAddr(0); // 本地摄像头
        ui->widgetVideo->open();
    }
    else
    {
        ui->btnOpenVideo->setText("打开视频");
        ui->widgetVideo->close();
    }
}

/**
 * @brief 串口中断函数
 */
void frmMain::slotSerialReadyRead()
{
    QByteArray array = pSerial->readAll();
//    QString str;
//    for (int i = 0; i < array.size(); i++) {
//        str += QString("%1 ").arg(static_cast<unsigned char>(array.at(i)), 0, 16, QChar('0'));
//    }
//    qDebug() << str;
    qDebug() << QString(array);
}

void frmMain::slotConfigChange(int index)
{
    QString group = QString("Config%1").arg(index+1);
    qDebug() << group;
    QSettings settings(CONFIG_FILEPATH, QSettings::IniFormat);
    if(index == 0)
    {
        int _1 = settings.value("Config1/Language").toInt();
        int _2 = settings.value("Config1/Theme").toInt();
        QString configSavePath = settings.value("Config1/configSavePath").toString();
        QString imageSavePath = settings.value("Config1/imageSavePath").toString();
        QString videoSavePath = settings.value("Config1/videoSavePath").toString();

        ui->cbLanguage->setCurrentIndex(_1);
        ui->cbTheme->setCurrentIndex(_2);
        ui->leSaveConfig->setText(configSavePath);
        ui->leSaveImage->setText(imageSavePath);
        ui->leSaveVideo->setText(videoSavePath);
    }
    else if(index == 1)
    {
        QSettings settings(CONFIG_FILEPATH, QSettings::IniFormat);
        QString port = settings.value("Config2/port").toString();
        int databit = settings.value("Config2/databit").toInt();
        int bandrate = settings.value("Config2/bandrate").toInt();
        int stopbit = settings.value("Config2/stopbit").toInt();
        int parity = settings.value("Config2/parity").toInt();
        QString videoaddr = settings.value("Config2/videoaddr").toString();

        ui->cbPorts->setCurrentText(port);
        ui->cbDatas->setCurrentIndex(databit);
        ui->cbBandrate->setCurrentIndex(bandrate);
        ui->cbStops->setCurrentIndex(stopbit);
        ui->cbParity->setCurrentIndex(parity);
        ui->leVideoAddr->setText(videoaddr);
    }
}

/**
 * @brief 控制机器人前进
 */
void frmMain::on_btnRobotUp_clicked()
{
    uint8_t send_msg[12] = {0};
    send_msg[0] = static_cast<unsigned char>(0xAA); // 帧头
    send_msg[1] = static_cast<unsigned char>(0x01); // 预留位
    send_msg[2] = static_cast<unsigned char>(0x01); //
    send_msg[3] = static_cast<unsigned char>(0x00); //
    send_msg[4] = static_cast<unsigned char>(0x00); //
    send_msg[5] = static_cast<unsigned char>(0x00); //
    send_msg[6] = static_cast<unsigned char>(0x00); //
    send_msg[7] = static_cast<unsigned char>(0x00); //
    send_msg[8] = static_cast<unsigned char>(0x00); //
    send_msg[9] = static_cast<unsigned char>(0x00); //
    send_msg[10] = static_cast<unsigned char>(checkNumber(send_msg, 10, 0)); //BBC校验位
    send_msg[11] = static_cast<unsigned char>(0xDD);

    pSerial->write(reinterpret_cast<char *>(send_msg), 12);
}

/**
 * @brief 控制机器人后退
 */
void frmMain::on_btnRobotDown_clicked()
{
//    short v1 = -272;
//    short highByte = (v1 & 0xFF00) >> 8;  // 高8位
//    short lowByte = v1 & 0x00FF;         // 低8位
//    struct RobotV v;
//    v.x.value = -150;
//    v.y.value = 0;
//    v.z.value = 0;

//    uint8_t send_msg[11] = {0};
//    send_msg[0] = static_cast<unsigned char>(0x7B); // 帧头
//    send_msg[1] = static_cast<unsigned char>(0x00); // 预留位
//    send_msg[2] = static_cast<unsigned char>(0x00); // 预留位
//    send_msg[3] = static_cast<unsigned char>(v.x.parts.highByte); // X轴速度高8位
//    send_msg[4] = static_cast<unsigned char>(v.x.parts.lowByte); //X轴速度低8位
//    send_msg[5] = static_cast<unsigned char>(v.y.parts.highByte); //Y轴速度高8位
//    send_msg[6] = static_cast<unsigned char>(v.y.parts.lowByte); //Y轴速度低8位
//    send_msg[7] = static_cast<unsigned char>(v.z.parts.highByte); //Z轴速度高8位
//    send_msg[8] = static_cast<unsigned char>(v.z.parts.lowByte); //Z轴速度低8位
//    send_msg[9] = static_cast<unsigned char>(checkNumber(send_msg, 9, 0)); //BBC校验位
//    send_msg[10] = static_cast<unsigned char>(0x7D);

//    pSerial->write(reinterpret_cast<char *>(send_msg), 11);

    uint8_t send_msg[12] = {0};
    send_msg[0] = static_cast<unsigned char>(0xAA); // 帧头
    send_msg[1] = static_cast<unsigned char>(0x01); // 预留位
    send_msg[2] = static_cast<unsigned char>(0x02); //
    send_msg[3] = static_cast<unsigned char>(0x00); //
    send_msg[4] = static_cast<unsigned char>(0x00); //
    send_msg[5] = static_cast<unsigned char>(0x00); //
    send_msg[6] = static_cast<unsigned char>(0x00); //
    send_msg[7] = static_cast<unsigned char>(0x00); //
    send_msg[8] = static_cast<unsigned char>(0x00); //
    send_msg[9] = static_cast<unsigned char>(0x00); //
    send_msg[10] = static_cast<unsigned char>(checkNumber(send_msg, 10, 0)); //BBC校验位
    send_msg[11] = static_cast<unsigned char>(0xDD);

    pSerial->write(reinterpret_cast<char *>(send_msg), 12);
}

/**
 * @brief 控制机器人左移
 */
void frmMain::on_btnRobotLeft_clicked()
{
    uint8_t send_msg[12] = {0};
    send_msg[0] = static_cast<unsigned char>(0xAA); // 帧头
    send_msg[1] = static_cast<unsigned char>(0x01); // 预留位
    send_msg[2] = static_cast<unsigned char>(0x08); //
    send_msg[3] = static_cast<unsigned char>(0x00); //
    send_msg[4] = static_cast<unsigned char>(0x00); //
    send_msg[5] = static_cast<unsigned char>(0x00); //
    send_msg[6] = static_cast<unsigned char>(0x00); //
    send_msg[7] = static_cast<unsigned char>(0x00); //
    send_msg[8] = static_cast<unsigned char>(0x00); //
    send_msg[9] = static_cast<unsigned char>(0x00); //
    send_msg[10] = static_cast<unsigned char>(checkNumber(send_msg, 10, 0)); //BBC校验位
    send_msg[11] = static_cast<unsigned char>(0xDD);

    pSerial->write(reinterpret_cast<char *>(send_msg), 12);
}

/**
 * @brief 控制机器人右移
 */
void frmMain::on_btnRobotRight_clicked()
{
    uint8_t send_msg[12] = {0};
    send_msg[0] = static_cast<unsigned char>(0xAA); // 帧头
    send_msg[1] = static_cast<unsigned char>(0x01); // 预留位
    send_msg[2] = static_cast<unsigned char>(0x07); //
    send_msg[3] = static_cast<unsigned char>(0x00); //
    send_msg[4] = static_cast<unsigned char>(0x00); //
    send_msg[5] = static_cast<unsigned char>(0x00); //
    send_msg[6] = static_cast<unsigned char>(0x00); //
    send_msg[7] = static_cast<unsigned char>(0x00); //
    send_msg[8] = static_cast<unsigned char>(0x00); //
    send_msg[9] = static_cast<unsigned char>(0x00); //
    send_msg[10] = static_cast<unsigned char>(checkNumber(send_msg, 10, 0)); //BBC校验位
    send_msg[11] = static_cast<unsigned char>(0xDD);

    pSerial->write(reinterpret_cast<char *>(send_msg), 12);
}

/**
 * @brief 控制机器人停止
 */
void frmMain::on_btnRobotStop_clicked()
{
    uint8_t send_msg[12] = {0};
    send_msg[0] = static_cast<unsigned char>(0xAA); // 帧头
    send_msg[1] = static_cast<unsigned char>(0x01); // 预留位
    send_msg[2] = static_cast<unsigned char>(0x00); //
    send_msg[3] = static_cast<unsigned char>(0x00); //
    send_msg[4] = static_cast<unsigned char>(0x00); //
    send_msg[5] = static_cast<unsigned char>(0x00); //
    send_msg[6] = static_cast<unsigned char>(0x00); //
    send_msg[7] = static_cast<unsigned char>(0x00); //
    send_msg[8] = static_cast<unsigned char>(0x00); //
    send_msg[9] = static_cast<unsigned char>(0x00); //
    send_msg[10] = static_cast<unsigned char>(checkNumber(send_msg, 10, 0)); //BBC校验位
    send_msg[11] = static_cast<unsigned char>(0xDD);

    pSerial->write(reinterpret_cast<char *>(send_msg), 12);
}

void frmMain::on_btnSaveConfig_clicked()
{
    QString filepath = QFileDialog::getOpenFileName(this, "选择配置文件", "./", "INI Files(*.ini)",nullptr,QFileDialog::DontUseNativeDialog);
    if(filepath.isEmpty()) return;
    ui->leSaveConfig->setText(filepath);
}

void frmMain::on_btnSaveImage_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "设置截图保存路径", "./", QFileDialog::DontUseNativeDialog);
    if(path.isEmpty()) return;

    ui->leSaveImage->setText(path);
}

void frmMain::on_btnSaveVideo_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "设置录像保存路径", "./", QFileDialog::DontUseNativeDialog);
    if(path.isEmpty()) return;

    ui->leSaveVideo->setText(path);
}

void frmMain::on_btnC1Cancel_clicked()
{
    slotConfigChange(0);
}


void frmMain::on_btnC1Apply_clicked()
{
    QString _configSavePath = ui->leSaveConfig->text();
    QString _imageSavePath = ui->leSaveImage->text();
    QString _videoSavePath = ui->leSaveVideo->text();

    QSettings settings(CONFIG_FILEPATH, QSettings::IniFormat);
    settings.beginGroup("Config1");
    settings.setValue("Language", ui->cbLanguage->currentIndex());
    settings.setValue("Theme", ui->cbTheme->currentIndex());
    settings.setValue("configSavePath", _configSavePath);
    settings.setValue("imageSavePath", _imageSavePath);
    settings.setValue("videoSavePath", _videoSavePath);
    settings.endGroup();

    if(settings.status() != QSettings::NoError)
    {
        QMessageBox::warning(this, "警告", "设置保存失败");
    }
    else
    {
        QMessageBox::information(this, "提示", "应用成功");
    }
}

void frmMain::on_btnC1Confirm_clicked()
{
    QString _configSavePath = ui->leSaveConfig->text();
    QString _imageSavePath = ui->leSaveImage->text();
    QString _videoSavePath = ui->leSaveVideo->text();

    QSettings settings(CONFIG_FILEPATH, QSettings::IniFormat);
    settings.beginGroup("Config1");
    settings.setValue("configSavePath", _configSavePath);
    settings.setValue("imageSavePath", _imageSavePath);
    settings.setValue("videoSavePath", _videoSavePath);
    settings.endGroup();

    if(settings.status() != QSettings::NoError)
    {
        QMessageBox::warning(this, "警告", "设置保存失败");
    }
    else
    {
        QMessageBox::information(this, "提示", "保存成功");
    }
}

void frmMain::on_btnC2Cancel_clicked()
{
    slotConfigChange(1);
}

void frmMain::on_btnC2Confirm_clicked()
{
    QString port = ui->cbPorts->currentText();
    int databit = ui->cbDatas->currentIndex();
    int bandrate = ui->cbBandrate->currentIndex();
    int stopbit = ui->cbStops->currentIndex();
    int parity = ui->cbParity->currentIndex();
    QString videoaddr = ui->leVideoAddr->text();

    QSettings settings(CONFIG_FILEPATH, QSettings::IniFormat);
    settings.beginGroup("Config2");
    settings.setValue("port", port);
    settings.setValue("databit", QString::number(databit));
    settings.setValue("bandrate", QString::number(bandrate));
    settings.setValue("stopbit", QString::number(stopbit));
    settings.setValue("parity", QString::number(parity));
    settings.setValue("videoaddr", videoaddr);
    settings.endGroup();
    if(settings.status() != QSettings::NoError)
    {
        QMessageBox::warning(this, "警告", "设置保存失败");
    }
    else
    {
        QMessageBox::information(this, "提示", "保存成功");
    }
}

void frmMain::on_btnRefreshPort_clicked()
{
    ui->cbPorts->clear();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) /* 初始化串口列表 */
    {
        ui->cbPorts->addItem(info.portName());
    }
}



void frmMain::on_btnOpenSerial_clicked()
{
    if(ui->btnOpenSerial->text() == "打开通信")
    {
        pSerial->setPortName(ui->cbPorts->currentText());
        if(!pSerial->open(QIODevice::ReadWrite))
        {
            QMessageBox* msgBox = new QMessageBox(this);

            msgBox->setAttribute(Qt::WA_DeleteOnClose);
            msgBox->setWindowTitle(tr("警告"));
            msgBox->setText(pSerial->errorString());
            msgBox->setIconPixmap(QPixmap(":/image/main_exit.png").scaled(100,100));
            QPushButton* yesButton = msgBox->addButton(tr("确认"), QMessageBox::AcceptRole);
            msgBox->setDefaultButton(yesButton);
            msgBox->exec();
            return;
        }
        ui->btnOpenSerial->setText("断开通信");

        pStatusWidget->setShowMessage("提示", "打开串口成功");
        pStatusWidget->start();

        LogManager::instance().getLogger()->info("打开串口成功");
    }
    else
    {
        if(!pSerial->isOpen()) return;
        pSerial->close();
        ui->btnOpenSerial->setText("打开通信");
        LogManager::instance().getLogger()->warn("关闭串口");
    }
}



void frmMain::on_tbFilter_clicked()
{
    int level = ui->cbLogLevel->currentIndex();
    LoggerSql::LogLevel _level = static_cast<LoggerSql::LogLevel>(level);
    QList<QVariantMap> data = plogSql->getLogs(_level);
    appendDatat2LogWidget(data);
}

void frmMain::on_tbSelectLogFile_clicked()
{

}
