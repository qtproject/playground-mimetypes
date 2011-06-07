INCLUDEPATH *= $$PWD/../include
DEPENDPATH  *= $$PWD

DEFINES += QT_NO_CAST_FROM_ASCII

SOURCES += qmimedatabase.cpp \
    qmimetype.cpp \
    magicmatcher.cpp \
    mimetypeparser.cpp \
    qmimemagicrule.cpp

HEADERS += qmime_global.h \
    qmimedatabase.h \
    qmimetype.h \
    magicmatcher.h \
    qmimetype_p.h \
    magicmatcher_p.h \
    mimetypeparser_p.h \
    qmimedatabase_p.h \
    qmimemagicrule.h
