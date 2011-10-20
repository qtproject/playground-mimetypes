QT       += testlib

QT       += widgets gui

TARGET = tst_qmimedatabase
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -L$$OUT_PWD/../../../src/mimetypes -lQtMimeTypes

INCLUDEPATH *= $$PWD/../../../include/QtMimeTypes

#DEFINES += SRC_DIR $$PWD

SOURCES += tst_qmimedatabase.cpp
DEFINES += SRCDIR='"\\"./\\""'

OTHER_FILES = testfiles/list
