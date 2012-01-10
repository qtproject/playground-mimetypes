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

contains(QMAKE_CXX, --sysroot): {
    unix:!symbian {
        maemo5 {
            DEFINES += SRCDIR='"\\"/opt/usr/lib/QtMimeTypes-tests/\\""'
        } else {
            DEFINES += SRCDIR='"\\"/usr/lib/QtMimeTypes-tests/\\""'
        }
    }
} else {
    DEFINES += SRCDIR='"\\"$$PWD/../\\""'

    QMAKE_EXTRA_TARGETS += check
    check.depends = $$TARGET
    check.commands = ./$$TARGET -xunitxml -o $${TARGET}.xml
}

QMAKE_CXXFLAGS += -W -Wall -Wextra -Werror -Wshadow -Wno-long-long -Wnon-virtual-dtor

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib/QtMimeTypes-tests/qmimedatabase-xml
    } else {
        target.path = /usr/lib/QtMimeTypes-tests/qmimedatabase-xml
    }
    INSTALLS += target
}
