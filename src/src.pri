QT += xml

INCLUDEPATH *= $$PWD/../include
DEPENDPATH  *= $$PWD

SOURCES += qmimedatabase.cpp \
    qmimetype.cpp \
    magicmatcher.cpp \
    mimetypeparser.cpp

HEADERS += qmimedatabase.h \
        qmimetype_global.h \
    qmimetype.h \
    magicmatcher.h \
    qmimetype_p.h \
    magicmatcher_p.h \
    qmimedatabase_p.h
