include(../../../../mimetypes.pri)

TEMPLATE = app

TARGET = tst_qmimedatabase-cache

QT       += testlib

QT       -= widgets gui

CONFIG   += console
CONFIG   -= app_bundle

CONFIG += depend_includepath

SOURCES += tst_qmimedatabase-cache.cpp
HEADERS += ../tst_qmimedatabase.h

DEFINES += SRCDIR='"\\"$$PWD/../\\""'

QMAKE_CXXFLAGS += -W -Wall -Wextra -Werror -Wshadow -Wno-long-long -Wnon-virtual-dtor
