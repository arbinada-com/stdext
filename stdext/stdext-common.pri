# QMake common settings for both library and test projects
#

# Set destination directories and makefile name
DESTDIR_SUBDIR = $${QT_ARCH}/$${QMAKE_PLATFORM}
CONFIG(debug, debug|release) {
    DESTDIR_SUBDIR = debug/$${QT_ARCH}/$${QMAKE_PLATFORM}
}
CONFIG(release, debug|release) {
    DESTDIR_SUBDIR = release/$${QT_ARCH}/$${QMAKE_PLATFORM}
}

DESTDIR = $${DESTDIR_SUBDIR}
OBJECTS_DIR = $${DESTDIR_SUBDIR}
MOC_DIR = $${DESTDIR_SUBDIR}/moc
RCC_DIR = $${DESTDIR_SUBDIR}/rcc
UI_DIR = $${DESTDIR_SUBDIR}/ui

MAKEFILE = Makefile-$${QT_ARCH}-$${QMAKE_PLATFORM}

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
