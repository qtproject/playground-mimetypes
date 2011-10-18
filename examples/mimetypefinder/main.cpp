#include "qmimedatabase.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc <= 1) {
        printf( "No filename specified\n" );
        return 1;
    }
    const QString fileName = QFile::decodeName(argv[1]);
    //int accuracy;
    QMimeDatabase db;
    QMimeType mime;
    if (fileName == QLatin1String("-")) {
        QFile qstdin;
        qstdin.open(stdin, QIODevice::ReadOnly);
        const QByteArray data = qstdin.readAll();
        //mime = QMimeType::findByContent(data, &accuracy);
        mime = db.findByData(data);
    //} else if (args->isSet("c")) {
        //mime = QMimeType::findByFileContent(fileName, &accuracy);
    } else {
        //mime = QMimeType::findByPath(fileName, 0, args->isSet("f"), &accuracy);
        mime = db.findByFile(fileName);
    }
    if ( mime.isValid() /*&& !mime.isDefault()*/ ) {
        printf("%s\n", mime.name().toLatin1().constData());
        //printf("(accuracy %d)\n", accuracy);
    } else {
        return 1; // error
    }

    return 0;
}
