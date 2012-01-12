include(../../../../mimetypes-nolibs.pri)
LIBS += -L$$OUT_PWD/../../../../src/mimetypes -lQtMimeTypes

TEMPLATE = app

TARGET = tst_qmimedatabase-cache

QT       += testlib

QT       -= widgets gui

CONFIG   += console
CONFIG   -= app_bundle

CONFIG += depend_includepath

SOURCES += tst_qmimedatabase-cache.cpp
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
    check.commands = LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:$$OUT_PWD/../../../../src/mimetypes ./$$TARGET -xunitxml -o $${TARGET}.xml
}

QMAKE_CXXFLAGS += -W -Wall -Wextra -Werror -Wshadow -Wno-long-long -Wnon-virtual-dtor

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib/QtMimeTypes-tests/qmimedatabase-cache
    } else {
        target.path = /usr/lib/QtMimeTypes-tests/qmimedatabase-cache
    }
    INSTALLS += target
}
