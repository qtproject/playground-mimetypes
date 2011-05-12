#-------------------------------------------------
#
# Project created by QtCreator 2011-05-12T12:38:51
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_qmimetest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -L$$OUT_PWD/../../lib -lqmimetype

INCLUDEPATH *= $$PWD/../../include

SOURCES += tst_qmimetest.cpp
#DEFINES += SRCDIR=\\\"$$PWD/\\\"
