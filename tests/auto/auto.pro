TEMPLATE = subdirs

SUBDIRS += \
    qmimetype \
    qmimedatabase \
    qdeclarativemimetype \
    qdeclarativemimedatabase

testrunner_config.files += tests.xml

unix:!symbian {
    maemo5 {
        testrunner_config.path = /opt/usr/share/QtMimeTypes-tests
    } else {
        testrunner_config.path = /usr/share/QtMimeTypes-tests
    }
    INSTALLS += testrunner_config
}
