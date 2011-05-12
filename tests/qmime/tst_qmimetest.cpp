#include <QtCore/QString>
#include <QFile>
#include <QtTest/QtTest>
#include <QMimeDatabase>

class QmimeTest : public QObject
{
    Q_OBJECT

public:
    QmimeTest();

private Q_SLOTS:
    void testFindByFile();
};

QmimeTest::QmimeTest()
{
}

void QmimeTest::testFindByFile()
{
    QMimeDatabase database;
    QVERIFY(database.addMimeTypes(SRCDIR"../../freedesktop.org.xml", 0));

    QFile testList(SRCDIR"testfiles/list");
    QVERIFY(testList.open(QIODevice::ReadOnly));

    while (!testList.atEnd()) {
        QString string = testList.readLine();
        string = string.trimmed();
        if (string.startsWith("#") || string.isEmpty())
            continue;

        QStringList list = string.split(" ", QString::SkipEmptyParts);
        QVERIFY(list.size() >= 2);
        bool failByFileData = false;
        if (list.size() == 3) {
            QString xxx = list.at(2);
            if (xxx.length() == 3) {
                failByFileData = xxx.at(2) == 'x';
            }
        }
        QString file = list.at(0);
        QString mimetype = list.at(1);
        file.prepend(SRCDIR"testfiles/");

        if (failByFileData) {
            QEXPECT_FAIL("", "Should fail", Continue);
            QCOMPARE(database.findByFile(QFileInfo(file)).type(), database.findByType(mimetype).type());
        } else
            QCOMPARE(database.findByFile(QFileInfo(file)).type(), database.findByType(mimetype).type());

    }
}

QTEST_APPLESS_MAIN(QmimeTest)

#include "tst_qmimetest.moc"
