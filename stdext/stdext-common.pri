# QMake common settings for both library and test projects
#

CONFIG -= qt
CONFIG -= app_bundle
CONFIG -= lex yacc

STDEXT_ARCH = $$QT_ARCH
STDEXT_PLATFORM = unknown
win32: STDEXT_PLATFORM = win32
win64: STDEXT_PLATFORM = win32
linux: STDEXT_PLATFORM = linux

STDEXT_CONF =
# Set destination directories
DESTDIR_SUBDIR = $${STDEXT_ARCH}/$${STDEXT_PLATFORM}
CONFIG(debug, debug|release) {
    DESTDIR_SUBDIR = debug/$${STDEXT_ARCH}/$${STDEXT_PLATFORM}
    STDEXT_CONF = Debug
}
CONFIG(release, debug|release) {
    DESTDIR_SUBDIR = release/$${STDEXT_ARCH}/$${STDEXT_PLATFORM}
    STDEXT_CONF = Release
}

DESTDIR = $${DESTDIR_SUBDIR}
OBJECTS_DIR = $${DESTDIR_SUBDIR}
MOC_DIR = $${DESTDIR_SUBDIR}/moc
RCC_DIR = $${DESTDIR_SUBDIR}/rcc
UI_DIR = $${DESTDIR_SUBDIR}/ui
#message(DESTDIR ($$TARGET): $$DESTDIR)

CONFIG(stdext_gen_makefile) {
    CONFIG -= qmake_use
    CONFIG -= qtc_run
    CONFIG -= debug_and_release
    CONFIG -= debug_and_release_target
    CONFIG += no_autoqmake
    # Set specific makefile name
    win32:MAKEFILE = Makefile-$${STDEXT_ARCH}-$${STDEXT_PLATFORM}.$${STDEXT_CONF}
    linux:MAKEFILE = Makefile-$${STDEXT_ARCH}-$${STDEXT_PLATFORM}.$${STDEXT_CONF}
    message(MAKEFILE: $$MAKEFILE)
}

win32 {
    CONFIG += c++17
}
unix {
    QMAKE_CXXFLAGS += -std=c++17 -pthread
}


# Copies the given file to the destination directory after link event
defineTest(PostLink_CopyFile) {
    src_file = $$1
    dest_dir = $$2
    # Replace slashes in paths with backslashes for Windows
    win32:src_file ~= s,/,\\,g
    win32:dest_dir ~= s,/,\\,g
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$src_file) $$quote($$dest_dir) $$escape_expand(\\n)
    export(QMAKE_POST_LINK)
}
