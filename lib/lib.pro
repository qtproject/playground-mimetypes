#-------------------------------------------------
#
# Project created by QtCreator 2011-05-07T00:45:27
#
#-------------------------------------------------

TARGET = qmimetype
TEMPLATE = lib

DEFINES += QMIME_LIBRARY

include($$PWD/../src/src.pri)

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
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
