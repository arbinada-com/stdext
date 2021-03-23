include(gtest-dependency.pri)
include(../stdext/stdext-common.pri)

TEMPLATE = app
CONFIG += console
CONFIG += thread

INCLUDEPATH += \
    ../stdext/src \
    ../stdext/src/json

LIBS_DIR = ../stdext/$${DESTDIR_SUBDIR}
win32: LIBS += -L$$LIBS_DIR/ -lstdext
unix: LIBS += -L$$LIBS_DIR/ -lstdext
DEPENDPATH += $$LIBS_DIR
win32: PRE_TARGETDEPS += $$LIBS_DIR/stdext.lib
unix: PRE_TARGETDEPS += $$LIBS_DIR/libstdext.a
#message(Uses libs: $$LIBS)
#message(Pre targetdeps: $$PRE_TARGETDEPS)

SOURCES += \
    src/containers_test.cpp \
    src/csvtools_test.cpp \
    src/datetime_test.cpp \
    src/ioutils_test.cpp \
    src/jsoncommon_test.cpp \
    src/jsondom_test.cpp \
    src/jsonlexer_test.cpp \
    src/jsonparser_test.cpp \
    src/jsontools_test.cpp \
    src/locutils_test.cpp \
    src/main.cpp \
    src/parsers_test.cpp \
    src/platforms_test.cpp \
    src/strutils_test.cpp \
    src/trees_test.cpp \
    src/variants_test.cpp

HEADERS += \
    src/jsondom_test.h \
    src/locutils_test.h

PostLink_CopyFile(./data/*.*, ./$$DESTDIR/)
