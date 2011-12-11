include(../../../mimetypes.pri)

TEMPLATE = app

TARGET = tst_qdeclarativemimedatabase

#CONFIG += warn_on qmltestcase
QT += qmltest

CONFIG += depend_includepath

SOURCES += tst_qdeclarativemimedatabase.cpp

# this reads the QML files from the same directory as this pro file
DEFINES += QUICK_TEST_SOURCE_DIR=\"\\\".\\\"\"
