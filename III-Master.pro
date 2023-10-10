QT += core gui serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

TARGET      = III-Master
TEMPLATE    = app

HEADERS     += head.h
SOURCES     += main.cpp
RESOURCES   += other/main.qrc
RESOURCES   += $$PWD/./core_qss/qss.qrc

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/form
include ($$PWD/form/form.pri)

INCLUDEPATH += $$PWD/./core_base
include ($$PWD/./core_base/core_base.pri)
include ($$PWD/video/video.pri)
include ($$PWD/statuswidget/statuswidget.pri)
include ($$PWD/log4qt/log4qt.pri)
DISTFILES +=

