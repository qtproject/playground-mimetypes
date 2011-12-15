include(../../../../mimetypes.pri)

TEMPLATE = app

TARGET = tst_qmimedatabase-xml

QT       += testlib

QT       -= widgets gui

CONFIG   += console
CONFIG   -= app_bundle

CONFIG += depend_includepath

SOURCES += tst_qmimedatabase-xml.cpp
HEADERS += ../tst_qmimedatabase.h

DEFINES += SRCDIR='"\\"/usr/lib/QtMimeTypes-tests/\\""'

QMAKE_CXXFLAGS += -W -Wall -Wextra -Werror -Wshadow -Wno-long-long -Wnon-virtual-dtor

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib/QtMimeTypes-tests/qmimedatabase-xml
    } else {
        target.path = /usr/lib/QtMimeTypes-tests/qmimedatabase-xml
    }
    INSTALLS += target
}
