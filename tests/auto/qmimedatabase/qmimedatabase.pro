TEMPLATE = subdirs
SUBDIRS = qmimedatabase-xml
unix: SUBDIRS += qmimedatabase-cache

OTHER_FILES = testfiles/list

the_testfiles.files += testfiles/*

unix:!symbian {
    maemo5 {
        the_testfiles.path = /opt/usr/lib/QtMimeTypes-tests/testfiles
    } else {
        the_testfiles.path = /usr/lib/QtMimeTypes-tests/testfiles
    }
    INSTALLS += the_testfiles
}
