CONFIG   += qt plugin
TEMPLATE = lib
TARGET   = declarative_mimetypes

# QtCore/qlist.h uses /usr/include/limits.h which uses does not compile with -pedantic.
# QtDeclarative/qdeclarativeprivate.h will not compile with -pedantic.
#MAKE_CXXFLAGS += -W -Wall -Wextra -Werror -ansi -pedantic -Wshadow -Wno-long-long -Wnon-virtual-dtor
QMAKE_CXXFLAGS += -W -Wall -Wextra -Werror -ansi           -Wshadow -Wno-long-long -Wnon-virtual-dtor
mac|darwin: {
} else {
    QMAKE_CXXFLAGS += -Wc++0x-compat
}


LIBS += -L../../mimetypes -lQtMimeTypes


INCLUDEPATH += ../../../include/QtMimeTypes ../../../src/mimetypes


SOURCES += mimetypes.cpp

# No headers


SOURCES += qdeclarativemimetype.cpp \
           qdeclarativemimedatabase.cpp

HEADERS += qdeclarativemimetype_p.h \
           qdeclarativemimedatabase_p.h


qmldir.files += $$PWD/qmldir


INSTALLS += qmldir target