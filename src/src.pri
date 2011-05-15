INCLUDEPATH *= $$PWD/../include
DEPENDPATH  *= $$PWD

SOURCES += qmimedatabase.cpp \
    qmimetype.cpp \
    magicmatcher.cpp \
    mimetypeparser.cpp \
    ../src/qmimemagicrule.cpp

HEADERS += qmime_global.h \
    qmimedatabase.h \
    qmimetype.h \
    magicmatcher.h \
    qmimetype_p.h \
    magicmatcher_p.h \
    qmimedatabase_p.h \
    ../src/qmimemagicrule.h
