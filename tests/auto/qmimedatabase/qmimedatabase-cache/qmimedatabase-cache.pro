QT       += testlib

QT       += widgets gui

TARGET = tst_qmimedatabase-cache
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -L$$OUT_PWD/../../../../src/mimetypes -lQtMimeTypes

INCLUDEPATH *= $$PWD/../../../../include/QtMimeTypes
INCLUDEPATH *= $$PWD/../../../../src/mimetypes/inqt5

SOURCES += tst_qmimedatabase-cache.cpp
HEADERS += ../tst_qmimedatabase.h
DEFINES += SRCDIR='"\\"$$PWD/../\\""'
