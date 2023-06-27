


HEADERS += $$PWD/OpencvThread.h
SOURCES += $$PWD/OpencvThread.cpp

INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/include/opencv2
LIBS += -L$$PWD/libs             \
               -lopencv_world452 \
