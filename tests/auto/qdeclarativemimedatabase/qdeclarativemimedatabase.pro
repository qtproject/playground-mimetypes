include(../../../mimetypes-nolibs.pri)
LIBS += -L$$OUT_PWD/../../../src/mimetypes -lQtMimeTypes

TEMPLATE = app

TARGET = tst_qdeclarativemimedatabase

#CONFIG += warn_on qmltestcase
CONFIG -= app_bundle

CONFIG += depend_includepath

SOURCES += tst_qdeclarativemimedatabase.cpp

# this reads the QML files from the same directory as this pro file
DEFINES += QUICK_TEST_SOURCE_DIR=\"\\\".\\\"\"

testcases.files += testdata_list.js tst_qdeclarativemimedatabase.qml

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib/QtMimeTypes-tests/qdeclarativemimedatabase
        testcases.path = /opt/usr/lib/QtMimeTypes-tests/qdeclarativemimedatabase
    } else {
        target.path = /usr/lib/QtMimeTypes-tests/qdeclarativemimedatabase
        testcases.path = /usr/lib/QtMimeTypes-tests/qdeclarativemimedatabase
    }
    INSTALLS += testcases target
}
