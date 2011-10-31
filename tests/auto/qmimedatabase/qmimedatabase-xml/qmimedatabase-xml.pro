QT       += testlib

QT       += widgets gui

TARGET = tst_qmimedatabase-xml
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -L$$OUT_PWD/../../../../src/mimetypes -lQtMimeTypes

INCLUDEPATH *= $$PWD/../../../../include/QtMimeTypes $$PWD/../../../../src/mimetypes/inqt5

SOURCES += tst_qmimedatabase-xml.cpp
HEADERS += ../tst_qmimedatabase.h
DEFINES += SRCDIR='"\\"$$PWD/../\\""'
