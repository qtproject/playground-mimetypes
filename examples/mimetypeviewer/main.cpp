#include "mimetypeviewer.h"

#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MimeTypeViewer v;
    v.show();

//    MainWindow w;
//    w.show();

    return a.exec();
}
