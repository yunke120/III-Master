
contains(QMAKE_CC, cl) {
    INCLUDEPATH += $$PWD/msvc64/include \
                   $$PWD/msvc64/include/log4qt

    LIBS += -L$$PWD/msvc64/libs -llog4qt
}

contains(QMAKE_CC, gcc) {
    INCLUDEPATH += $$PWD/mingw64/include \
                   $$PWD/mingw64/include/log4qt

    LIBS += -L$$PWD/mingw64/libs -lliblog4qt
}

HEADERS += \
    $$PWD/log.h
