

HEADERS += $$PWD/pythonthread.h
SOURCES += $$PWD/pythonthread.cpp

INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/include/python
INCLUDEPATH += $$PWD/include/numpy
LIBS += -L$$PWD/libs             \
                      -lpython3  \
                      -lpython38 \
