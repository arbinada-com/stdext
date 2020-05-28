# QMake common settings for both library and test projects
#

# Set destination directories and makefile name
DESTDIR_SUBDIR = $${QMAKE_PLATFORM}/$${QT_ARCH}
CONFIG(debug, debug|release) {
    DESTDIR_SUBDIR = debug/$${QMAKE_PLATFORM}/$${QT_ARCH}
}
CONFIG(release, debug|release) {
    DESTDIR_SUBDIR = release/$${QMAKE_PLATFORM}/$${QT_ARCH}
}

DESTDIR = $${DESTDIR_SUBDIR}
OBJECTS_DIR = $${DESTDIR_SUBDIR}
MOC_DIR = $${DESTDIR_SUBDIR}/moc
RCC_DIR = $${DESTDIR_SUBDIR}/rcc
UI_DIR = $${DESTDIR_SUBDIR}/ui

MAKEFILE = Makefile-$$QMAKE_PLATFORM-$$QT_ARCH
