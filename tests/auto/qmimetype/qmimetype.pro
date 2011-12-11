include(../../../mimetypes.pri)

TEMPLATE = app

TARGET   = tst_qmimetype
CONFIG   += qtestlib
DEPENDPATH += .

QMAKE_CXXFLAGS += -W -Wall -Wextra -Werror -Wshadow -Wno-long-long -Wnon-virtual-dtor

CONFIG += depend_includepath


SOURCES += tst_qmimetype.cpp

HEADERS += tst_qmimetype.h


QMAKE_EXTRA_TARGETS += check
check.depends = $$TARGET
check.commands = ./$$TARGET -xunitxml -o unitTestResults.xml
