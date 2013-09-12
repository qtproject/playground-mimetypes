!isEmpty(MIMETYPES_PRI_INCLUDED):error("mimetypes-no-libs.pri already included")
MIMETYPES_PRI_INCLUDED = 1

INCLUDEPATH += $$PWD/include/QtMimeTypes
INCLUDEPATH += $$PWD/src/mimetypes/inqt5
INCLUDEPATH += $$PWD/src/mimetypes

mac|darwin: {
    QMAKE_CXXFLAGS += -ansi
} else:false {
    QMAKE_CXXFLAGS += -ansi -Wc++0x-compat
} else {
    QMAKE_CXXFLAGS += -std=c++0x
}

unix {
    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    INCLUDEDIR = $${PREFIX}/include
    LIBDIR = $${PREFIX}/lib
}
