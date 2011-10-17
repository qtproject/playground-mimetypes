CONFIG   += qtestlib
TEMPLATE = app
TARGET   = tst_qmimetype
DEPENDPATH += .

QMAKE_CXXFLAGS += -W -Wall -Wextra -Werror -ansi -pedantic -Wshadow -Wno-long-long -Wnon-virtual-dtor
mac|darwin: {
} else {
    QMAKE_CXXFLAGS += -Wc++0x-compat
}

# runtime environment
LIBS += -L../../../src/mimetypes -lQtMimeTypes


API_DIR = ..

INCLUDEPATH += ../../../include/QtMimeTypes ../../../src/mimetypes $$API_DIR/unittests


SOURCES += tst_qmimetype.cpp

HEADERS += tst_qmimetype.h


QMAKE_EXTRA_TARGETS += check
check.depends = $$TARGET
check.commands = ./$$TARGET -xunitxml -o unitTestResults.xml
