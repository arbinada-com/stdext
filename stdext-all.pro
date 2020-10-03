TEMPLATE = subdirs
SUBDIRS = stdext \
        stdext-test

stdext-test.depends = stdext
