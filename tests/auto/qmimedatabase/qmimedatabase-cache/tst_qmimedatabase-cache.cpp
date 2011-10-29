#include <../tst_qmimedatabase.h>
#include <QDir>
#include <QFile>
#include <QtTest>
#include <qstandardpaths.h>

#include "../tst_qmimedatabase.cpp"

tst_qmimedatabase::tst_qmimedatabase()
{
    qputenv("XDG_DATA_HOME", QByteArray("doesnotexist"));
    // Copy SRCDIR "../../../src/mimetypes/mime to a temp dir
    // then run update-mime-database
    // then set XDG_DATA_DIRS to the TEMP dir

    QDir here = QDir::currentPath();
    here.mkpath("mime/packages");
    QFile xml(SRCDIR "../../../src/mimetypes/mime/packages/freedesktop.org.xml");
    const QString tempMime = here.absolutePath() + "/mime";
    xml.copy(tempMime + "/packages/freedesktop.org.xml");

    const QString umd = QStandardPaths::findExecutable("update-mime-database");
    if (umd.isEmpty())
        QSKIP("shared-mime-info not found, skipping mime.cache test", SkipAll);

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels); // silence output
    proc.start(umd, QStringList() << tempMime);
    proc.waitForFinished();

    QVERIFY(QFile::exists(tempMime + "/mime.cache"));
    qputenv("XDG_DATA_DIRS", QFile::encodeName(here.absolutePath()));
}
