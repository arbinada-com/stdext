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
