#include "../tst_qmimedatabase.h"
#include <QDir>
#include <QFile>
#include <QtTest/QtTest>
#include <qstandardpaths.h>

#include "../tst_qmimedatabase.cpp"

tst_qmimedatabase::tst_qmimedatabase()
{
    QDir here = QDir::currentPath();
    const QString tempMime = here.absolutePath() + QString::fromLatin1("/mime");
    runUpdateMimeDatabase(tempMime);
    QVERIFY(QFile::exists(tempMime + QString::fromLatin1("/mime.cache")));
}
