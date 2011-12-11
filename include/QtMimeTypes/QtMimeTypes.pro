TEMPLATE = aux

the_includes.files += QMimeDatabase \
                      QMimeType \

unix:!symbian {
    maemo5 {
        the_includes.path = /opt/usr/include/qt5/QtMimeTypes
    } else {
        the_includes.path = /usr/include/qt5/QtMimeTypes
    }
    INSTALLS += the_includes
}
