include(../../../mimetypes.pri)

TEMPLATE = app

TARGET = tst_qdeclarativemimetype

#CONFIG += warn_on qmltestcase
CONFIG -= app_bundle

QT += qmltest

CONFIG += depend_includepath

SOURCES += tst_qdeclarativemimetype.cpp

# this reads the QML files from the same directory as this pro file
DEFINES += QUICK_TEST_SOURCE_DIR=\"\\\".\\\"\"

testcases.files += tst_qdeclarativemimetypedata.js tst_qdeclarativemimetypeproperties.qml tst_qdeclarativemimetype.qml

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib/QtMimeTypes-tests/qdeclarativemimetype
        testcases.path = /opt/usr/lib/QtMimeTypes-tests/qdeclarativemimetype
    } else {
        target.path = /usr/lib/QtMimeTypes-tests/qdeclarativemimetype
        testcases.path = /usr/lib/QtMimeTypes-tests/qdeclarativemimetype
    }
    INSTALLS += testcases target
}
