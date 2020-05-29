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
    src/containers-test.h \
    src/csvtools-test.h \
    src/datetime-test.h \
    src/ioutils-test.h \
    src/jsondom-test.h \
    src/jsonlexer-test.h \
    src/jsonparser-test.h \
    src/jsontools-test.h \
    src/locutils-test.h \
    src/parsers-test.h \
    src/platforms-test.h \
    src/strutils-test.h \
    src/trees-test.h \
    src/variants-test.h

SOURCES += \
    src/main.cpp

PostLink_CopyFile($$PWD/data/*, $$PWD/$$DESTDIR/*)
