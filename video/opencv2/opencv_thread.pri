


HEADERS += $$PWD/opencvthread.h
SOURCES += $$PWD/opencvthread.cpp

INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/include/opencv2
LIBS += -L$$PWD/libs             \
               -lopencv_world452 \
