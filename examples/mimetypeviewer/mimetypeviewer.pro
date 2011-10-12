#-------------------------------------------------
#
# Project created by QtCreator 2011-05-07T14:29:03
#
#-------------------------------------------------

QT       += core gui

QT += widgets

TARGET = MimeTypeViewer
TEMPLATE = app

message($$PWD)

LIBS += -L$$OUT_PWD/../../src/mimetypes -lQtMimeTypes

INCLUDEPATH *= $$PWD/../../include/QtMimeTypes

SOURCES += main.cpp\
    mimetypeviewer.cpp

HEADERS  += mimetypeviewer.h

FORMS    += mimetypeviewer.ui
