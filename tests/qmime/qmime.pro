#-------------------------------------------------
#
# Project created by QtCreator 2011-05-12T12:38:51
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_qmimetype
CONFIG   += console
CONFIG   -= app_bundle

message($$PWD)

TEMPLATE = app

LIBS += -L$$OUT_PWD/../../src/mimetypes -lQtMimeTypes

INCLUDEPATH *= $$PWD/../../include/QtMimeTypes

#DEFINES += SRC_DIR $$PWD

SOURCES += tst_qmimetype.cpp
DEFINES += SRCDIR='"\\"$$PWD/\\""'

OTHER_FILES = testfiles/list
