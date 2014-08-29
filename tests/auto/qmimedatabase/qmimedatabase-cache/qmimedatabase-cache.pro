include(../../../../mimetypes-nolibs.pri)
LIBS += -L$$OUT_PWD/../../../../src/mimetypes -lQtMimeTypes

TEMPLATE = app

TARGET = tst_qmimedatabase-cache

QT       += testlib

QT       -= widgets gui

CONFIG   += console
CONFIG   -= app_bundle

CONFIG += depend_includepath

SOURCES = tst_qmimedatabase-cache.cpp
HEADERS = ../tst_qmimedatabase.h

DEFINES += SRCDIR='"\\"$$PWD/../\\""'

QMAKE_EXTRA_TARGETS += check
check.depends = $$TARGET
check.commands = LD_LIBRARY_PATH=$$(LD_LIBRARY_PATH):$$OUT_PWD/../../../../src/mimetypes ./$$TARGET

DEFINES += CORE_SOURCES='"\\"$$PWD/../../../../src\\""'

*-g++*:QMAKE_CXXFLAGS += -W -Wall -Wextra -Wnon-virtual-dtor
