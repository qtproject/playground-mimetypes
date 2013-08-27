include(../../mimetypes-nolibs.pri)

TARGET = QtMimeTypes
TEMPLATE = lib

win32: DESTDIR = ./

DEFINES += QMIME_LIBRARY

DEPENDPATH  *= $$PWD

CONFIG += depend_includepath

QT     = core

QMAKE_CXXFLAGS += -W -Wall -Wextra -Werror -Wshadow -Wno-long-long -Wnon-virtual-dtor

SOURCES += qmimedatabase.cpp \
           qmimetype.cpp \
           qmimemagicrulematcher.cpp \
           qmimetypeparser.cpp \
           qmimemagicrule.cpp \
           qmimeglobpattern.cpp \
           qmimeprovider.cpp

the_includes.files += qmime_global.h \
                      qmimedatabase.h \
                      qmimetype.h \

HEADERS += $$the_includes.files \
           qmimemagicrulematcher_p.h \
           qmimetype_p.h \
           qmimetypeparser_p.h \
           qmimedatabase_p.h \
           qmimemagicrule_p.h \
           qmimeglobpattern_p.h \
           qmimeprovider_p.h

SOURCES += inqt5/qstandardpaths.cpp
win32: SOURCES += inqt5/qstandardpaths_win.cpp
unix: {
    macx-*: {
        SOURCES += inqt5/qstandardpaths_mac.cpp
        LIBS += -framework Carbon
    } else {
        SOURCES += inqt5/qstandardpaths_unix.cpp
    }
}

RESOURCES += \
    mimetypes.qrc

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEA6A790B
    TARGET.CAPABILITY =
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = lib.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    target.path = ${{LIBDIR}
    the_includes.path = $${INCLUDEDIR}/QtMimeTypes
    INSTALLS += target the_includes
}
