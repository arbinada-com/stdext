include(gtest-dependency.pri)
include(../stdext/stdext-common.pri)

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG += thread
CONFIG -= qt

INCLUDEPATH += ../stdext/src

LIBS_DIR = $$PWD/../stdext/$${DESTDIR_SUBDIR}
win32: LIBS += -L$$LIBS_DIR/ -lstdext
unix: LIBS += -L$$LIBS_DIR/ -lstdext
DEPENDPATH += $$LIBS_DIR
win32: PRE_TARGETDEPS += $$LIBS_DIR/stdext.lib
unix: PRE_TARGETDEPS += $$LIBS_DIR/stdext.a


HEADERS += \
    src/datetime-test.h

SOURCES += \
    src/main.cpp
