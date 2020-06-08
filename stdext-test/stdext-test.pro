include(gtest-dependency.pri)
include(../stdext/stdext-common.pri)

TEMPLATE = app
CONFIG += console c++11
CONFIG += thread

INCLUDEPATH += \
    ../stdext/src \
    ../stdext/src/json

LIBS_DIR = $$PWD/../stdext/$${DESTDIR_SUBDIR}
win32: LIBS += -L$$LIBS_DIR/ -lstdext
unix: LIBS += -L$$LIBS_DIR/ -lstdext
DEPENDPATH += $$LIBS_DIR
win32: PRE_TARGETDEPS += $$LIBS_DIR/stdext.lib
unix: PRE_TARGETDEPS += $$LIBS_DIR/libstdext.a
#message(Uses libs: $$LIBS)
#message(Pre targetdeps: $$PRE_TARGETDEPS)

SOURCES += \
    src/main.cpp \
    src/containers-test.cpp \
    src/csvtools-test.cpp \
    src/datetime-test.cpp \
    src/ioutils-test.cpp \
    src/jsondom-test.cpp \
    src/jsonlexer-test.cpp \
    src/jsonparser-test.cpp \
    src/jsontools-test.cpp \
    src/locutils-test.cpp \
    src/parsers-test.cpp \
    src/platforms-test.cpp \
    src/strutils-test.cpp \
    src/trees-test.cpp \
    src/variants-test.cpp

PostLink_CopyFile($$PWD/data/*.*, $$PWD/$$DESTDIR/)
