include(stdext-common.pri)

TEMPLATE = lib
TARGET = stdext
CONFIG += staticlib

win32 {
    contains(QMAKE_CXX, cl) {
        # Up MSVC warning level
        QMAKE_CXXFLAGS_WARN_ON -= -W3
        QMAKE_CXXFLAGS_WARN_ON += -W4
    }
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/csvtools.cpp \
    src/datetime.cpp \
    src/ioutils.cpp \
    src/json/jsoncommon.cpp \
    src/json/jsondom.cpp \
    src/json/jsonlexer.cpp \
    src/json/jsonparser.cpp \
    src/json/jsontools.cpp \
    src/locutils.cpp \
    src/parsers.cpp \
    src/strutils.cpp \
    src/testutils.cpp \
    src/variants.cpp

HEADERS += \
    src/csvtools.h \
    src/datetime.h \
    src/ioutils.h \
    src/json/json.h \
    src/json/jsoncommon.h \
    src/json/jsondom.h \
    src/json/jsonexceptions.h \
    src/json/jsonlexer.h \
    src/json/jsonparser.h \
    src/json/jsontools.h \
    src/locutils.h \
    src/parsers.h \
    src/platforms.h \
    src/ptr_vector.h \
    src/strutils.h \
    src/testutils.h \
    src/trees.h \
    src/variants.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target
