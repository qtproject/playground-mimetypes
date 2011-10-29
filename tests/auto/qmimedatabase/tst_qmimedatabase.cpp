#include "tst_qmimedatabase.h"

#include <QtCore/QFile>

#include <QtTest/QtTest>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets/QIcon>
#else
#include <QtGui/QIcon>
#endif

#include <QMimeDatabase>

tst_qmimedatabase::~tst_qmimedatabase()
{
}

void tst_qmimedatabase::initTestCase()
{
}

void tst_qmimedatabase::test_mimeTypeForName()
{
    QMimeDatabase db;
    QMimeType s0 = db.mimeTypeForName("application/x-zerosize");
    QVERIFY(s0.isValid());
    QCOMPARE(s0.name(), QString::fromLatin1("application/x-zerosize"));
    QCOMPARE(s0.comment(), QString::fromLatin1("empty document"));

    QMimeType s0Again = db.mimeTypeForName("application/x-zerosize");
    QCOMPARE(s0Again.name(), s0.name());

    QMimeType s1 = db.mimeTypeForName("text/plain");
    QVERIFY(s1.isValid());
    QCOMPARE(s1.name(), QString::fromLatin1("text/plain"));
    //qDebug("Comment is %s", qPrintable(s1.comment()));

    QMimeType krita = db.mimeTypeForName("application/x-krita");
    QVERIFY(krita.isValid());

    // Test <comment> parsing with application/rdf+xml which has the english comment after the other ones
    QMimeType rdf = db.mimeTypeForName("application/rdf+xml");
    QVERIFY(rdf.isValid());
    QCOMPARE(rdf.comment(), QString::fromLatin1("RDF file"));

    QMimeType bzip2 = db.mimeTypeForName("application/x-bzip2");
    QVERIFY(bzip2.isValid());
    QCOMPARE(bzip2.comment(), QString::fromLatin1("Bzip archive"));

    QMimeType defaultMime = db.mimeTypeForName("application/octet-stream");
    QVERIFY(defaultMime.isValid());
    QVERIFY(defaultMime.isDefault());

    // TODO move to test_findByFile
#ifdef Q_OS_LINUX
    QString exePath = QStandardPaths::findExecutable("ls");
    if (exePath.isEmpty())
        qWarning() << "ls not found";
    else {
        const QString executableType = QString::fromLatin1("application/x-executable");
        //QTest::newRow("executable") << exePath << executableType;
        QCOMPARE(db.findByFile(exePath).name(), executableType);
    }
#endif

}

void tst_qmimedatabase::test_findByName_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("expectedMimeType");
    // Maybe we could also add a expectedAccuracy column...

    QTest::newRow("text") << "textfile.txt" << "text/plain";
    QTest::newRow("case-insensitive search") << "textfile.TxT" << "text/plain";

    // Needs shared-mime-info > 0.91. Earlier versions wrote .Z to the mime.cache file...
    //QTest::newRow("case-insensitive match on a non-lowercase glob") << "foo.z" << "application/x-compress";

    QTest::newRow("case-sensitive uppercase match") << "textfile.C" << "text/x-c++src";
    QTest::newRow("case-sensitive lowercase match") << "textfile.c" << "text/x-csrc";
    QTest::newRow("case-sensitive long-extension match") << "foo.PS.gz" << "application/x-gzpostscript";
    QTest::newRow("case-sensitive-only match") << "core" << "application/x-core";
    QTest::newRow("case-sensitive-only match") << "Core" << "application/octet-stream"; // #198477

    QTest::newRow("desktop file") << "foo.desktop" << "application/x-desktop";
    QTest::newRow("old kdelnk file is x-desktop too") << "foo.kdelnk" << "application/x-desktop";
    QTest::newRow("double-extension file") << "foo.tar.bz2" << "application/x-bzip-compressed-tar";
    QTest::newRow("single-extension file") << "foo.bz2" << "application/x-bzip";
    QTest::newRow(".doc should assume msword") << "somefile.doc" << "application/msword"; // #204139
    QTest::newRow("glob that uses [] syntax, 1") << "Makefile" << "text/x-makefile";
    QTest::newRow("glob that uses [] syntax, 2") << "makefile" << "text/x-makefile";
    QTest::newRow("glob that ends with *, no extension") << "README" << "text/x-readme";
    QTest::newRow("glob that ends with *, extension") << "README.foo" << "text/x-readme";
    QTest::newRow("glob that ends with *, also matches *.txt. Higher weight wins.") << "README.txt" << "text/plain";
    QTest::newRow("glob that ends with *, also matches *.nfo. Higher weight wins.") << "README.nfo" << "text/x-nfo";
    // fdo bug 15436, needs shared-mime-info >= 0.40 (and this tests the globs2-parsing code).
    QTest::newRow("glob that ends with *, also matches *.pdf. *.pdf has higher weight") << "README.pdf" << "application/pdf";
    QTest::newRow("directory") << "/" << "inode/directory";
    QTest::newRow("doesn't exist, no extension") << "IDontExist" << "application/octet-stream";
    QTest::newRow("doesn't exist but has known extension") << "IDontExist.txt" << "text/plain";
}

void tst_qmimedatabase::test_findByName()
{
    QFETCH(QString, fileName);
    QFETCH(QString, expectedMimeType);
    QMimeDatabase db;
    QMimeType mime = db.findByName(fileName);
    QVERIFY(mime.isValid());
    QCOMPARE(mime.name(), expectedMimeType);
}

void tst_qmimedatabase::test_inheritance()
{
    QMimeDatabase db;

    // All file-like mimetypes inherit from octet-stream
    const QMimeType wordperfect = db.mimeTypeForName("application/vnd.wordperfect");
    QVERIFY(wordperfect.isValid());
    QCOMPARE(wordperfect.parentMimeTypes().join(","), QString("application/octet-stream"));
    QVERIFY(wordperfect.inherits("application/octet-stream"));

    QVERIFY(db.mimeTypeForName("image/svg+xml-compressed").inherits("application/x-gzip"));

    // Check that msword derives from ole-storage
    const QMimeType msword = db.mimeTypeForName("application/msword");
    QVERIFY(msword.isValid());
    const QMimeType olestorage = db.mimeTypeForName("application/x-ole-storage");
    QVERIFY(olestorage.isValid());
    QVERIFY(msword.inherits(olestorage.name()));
    QVERIFY(msword.inherits("application/octet-stream"));

    const QMimeType directory = db.mimeTypeForName("inode/directory");
    QVERIFY(directory.isValid());
    QCOMPARE(directory.parentMimeTypes().count(), 0);
    QVERIFY(!directory.inherits("application/octet-stream"));

    // Check that text/x-patch knows that it inherits from text/plain (it says so explicitly)
    const QMimeType plain = db.mimeTypeForName("text/plain");
    const QMimeType derived = db.mimeTypeForName("text/x-patch");
    QVERIFY(derived.isValid());
    QCOMPARE(derived.parentMimeTypes().join(","), plain.name());
    QVERIFY(derived.inherits("text/plain"));
    QVERIFY(derived.inherits("application/octet-stream"));

    // Check that application/x-shellscript inherits from application/x-executable
    // (Otherwise KRun cannot start shellscripts...)
    // This is a test for multiple inheritance...
    const QMimeType shellscript = db.mimeTypeForName("application/x-shellscript");
    QVERIFY(shellscript.isValid());
    QVERIFY(shellscript.inherits("text/plain"));
    QVERIFY(shellscript.inherits("application/x-executable"));
    const QStringList shellParents = shellscript.parentMimeTypes();
    QVERIFY(shellParents.contains("text/plain"));
    QVERIFY(shellParents.contains("application/x-executable"));
    QCOMPARE(shellParents.count(), 2); // only the above two
    const QStringList allShellParents = shellscript.allParentMimeTypes();
    QVERIFY(allShellParents.contains("text/plain"));
    QVERIFY(allShellParents.contains("application/x-executable"));
    QVERIFY(allShellParents.contains("application/octet-stream"));
    // Must be least-specific last, i.e. breadth first.
    QCOMPARE(allShellParents.last(), QString("application/octet-stream"));

    // Check that text/x-mrml knows that it inherits from text/plain (implicitly)
    const QMimeType mrml = db.mimeTypeForName("text/x-mrml");
    QVERIFY(mrml.isValid());
    QVERIFY(mrml.inherits("text/plain"));
    QVERIFY(mrml.inherits("application/octet-stream"));

    // Check that msword-template inherits msword
    const QMimeType mswordTemplate = db.mimeTypeForName("application/msword-template");
    QVERIFY(mswordTemplate.isValid());
    QVERIFY(mswordTemplate.inherits("application/msword"));
}

void tst_qmimedatabase::test_aliases()
{
    QMimeDatabase db;

    const QMimeType canonical = db.mimeTypeForName("application/xml");
    QVERIFY(canonical.isValid());

    QMimeType resolvedAlias = db.mimeTypeForName("text/xml");
    QVERIFY(resolvedAlias.isValid());
    QCOMPARE(resolvedAlias.name(), QString("application/xml"));

    QVERIFY(resolvedAlias.inherits("application/xml"));
    QVERIFY(canonical.inherits("text/xml"));

    // Test for kde bug 197346: does nspluginscan see that audio/mp3 already exists?
    bool mustWriteMimeType = !db.mimeTypeForName("audio/mp3").isValid();
    QVERIFY(!mustWriteMimeType);
}

void tst_qmimedatabase::findByName_data()
{
    QTest::addColumn<QString>("filePath");
    QTest::addColumn<QString>("mimeTypeName");
    QTest::addColumn<QString>("xFail");

    QString prefix = QLatin1String(SRCDIR "testfiles/");

    QFile f(prefix + QLatin1String("list"));
    QVERIFY2(f.open(QIODevice::ReadOnly), qPrintable(prefix));

    QByteArray line(1024, Qt::Uninitialized);

    while (!f.atEnd()) {
        int len = f.readLine(line.data(), 1023);

        if (len <= 2 || line.at(0) == '#')
            continue;

        QString string = QString::fromLatin1(line.constData(), len - 1).trimmed();
        QStringList list = string.split(QLatin1Char(' '), QString::SkipEmptyParts);
        QVERIFY(list.size() >= 2);

        QString filePath = list.at(0);
        QString mimeTypeType = list.at(1);
        QString xFail;
        if (list.size() >= 3)
            xFail = list.at(2);

        QTest::newRow(filePath.toLatin1().constData()) << prefix + filePath << mimeTypeType << xFail;
    }
}

void tst_qmimedatabase::findByName()
{
    QFETCH(QString, filePath);
    QFETCH(QString, mimeTypeName);
    QFETCH(QString, xFail);

    QMimeDatabase database;

    //qDebug() << Q_FUNC_INFO << filePath;

    const QMimeType resultMimeType(database.findByName(filePath));
    if (resultMimeType.isValid()) {
        //qDebug() << Q_FUNC_INFO << "MIME type" << resultMimeType.name() << "has generic icon name" << resultMimeType.genericIconName() << "and icon name" << resultMimeType.iconName();

// Loading icons depend on the icon theme, we can't enable this test
#if 0 // (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        QCOMPARE(resultMimeType.genericIconName(), QIcon::fromTheme(resultMimeType.genericIconName()).name());
        QVERIFY2(!QIcon::fromTheme(resultMimeType.genericIconName()).isNull(), qPrintable(resultMimeType.genericIconName()));
        QVERIFY2(QIcon::hasThemeIcon(resultMimeType.genericIconName()), qPrintable(resultMimeType.genericIconName()));

        QCOMPARE(resultMimeType.iconName(), QIcon::fromTheme(resultMimeType.iconName()).name());
        QVERIFY2(!QIcon::fromTheme(resultMimeType.iconName()).isNull(), qPrintable(resultMimeType.iconName()));
        QVERIFY2(QIcon::hasThemeIcon(resultMimeType.iconName()), qPrintable(resultMimeType.iconName()));
#else
        // Under Qt4 not all genericIconNames or iconNames return an icon that is valid.
#endif
    }
    const QString resultMimeTypeName = resultMimeType.name();
    //qDebug() << Q_FUNC_INFO << "findByName() returned" << resultMimeTypeName;

    const bool failed = resultMimeTypeName != mimeTypeName;
    const bool shouldFail = (xFail.length() >= 1 && xFail.at(0) == QLatin1Char('x'));
    if (shouldFail != failed) {
        // Results are ambiguous when multiple MIME types have the same glob
        // -> accept the current result if the found MIME type actually
        // matches the file's extension.
        // TODO: a better file format in testfiles/list!
        const QMimeType foundMimeType = database.mimeTypeForName(resultMimeTypeName);
        QVERIFY2(resultMimeType == foundMimeType, qPrintable(resultMimeType.name() + " vs. " + foundMimeType.name()));
        if (foundMimeType.isValid()) {
            const QString extension = QFileInfo(filePath).suffix();
            //qDebug() << Q_FUNC_INFO << "globPatterns:" << foundMimeType.globPatterns() << "- extension:" << QString() + "*." + extension;
            if (foundMimeType.globPatterns().contains("*." + extension))
                return;
        }
    }
    if (shouldFail) {
        // Expected to fail
        QVERIFY2(resultMimeTypeName != mimeTypeName, qPrintable(resultMimeTypeName));
    } else {
        QCOMPARE(resultMimeTypeName, mimeTypeName);
    }
}

void tst_qmimedatabase::findByData_data()
{
    findByName_data();
}

void tst_qmimedatabase::findByData()
{
    QFETCH(QString, filePath);
    QFETCH(QString, mimeTypeName);
    QFETCH(QString, xFail);

    QMimeDatabase database;
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QByteArray data = f.readAll();

    const QString resultMimeTypeName = database.findByData(data).name();
    if (xFail.length() >= 2 && xFail.at(1) == QLatin1Char('x')) {
        // Expected to fail
        QVERIFY2(resultMimeTypeName != mimeTypeName, qPrintable(resultMimeTypeName));
    } else {
        QCOMPARE(resultMimeTypeName, mimeTypeName);
    }
}

void tst_qmimedatabase::findByFile_data()
{
    findByName_data();
}

void tst_qmimedatabase::findByFile()
{
    QFETCH(QString, filePath);
    QFETCH(QString, mimeTypeName);
    QFETCH(QString, xFail);

    QMimeDatabase database;
    const QString resultMimeTypeName = database.findByFile(QFileInfo(filePath)).name();
    //qDebug() << Q_FUNC_INFO << filePath << "->" << resultMimeTypeName;
    if (xFail.length() >= 3 && xFail.at(2) == QLatin1Char('x')) {
        // Expected to fail
        QVERIFY2(resultMimeTypeName != mimeTypeName, qPrintable(resultMimeTypeName));
    } else {
        QCOMPARE(resultMimeTypeName, mimeTypeName);
    }
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
//QTEST_MAIN(tst_qmimedatabase)
QTEST_APPLESS_MAIN(tst_qmimedatabase)
#else
// If tests with icons were activated in Qt4 we'd use QTEST_MAIN:
//QTEST_MAIN(tst_qmimedatabase)
QTEST_APPLESS_MAIN(tst_qmimedatabase)
#endif
