
QT       += core

TEMPLATE = app

LIBS += -L$$OUT_PWD/../../src/mimetypes -lQtMimeTypes

INCLUDEPATH *= $$PWD/../../include/QtMimeTypes

SOURCES += main.cpp

