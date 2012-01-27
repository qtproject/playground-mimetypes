/****************************************************************************
**
** TODO Provide Licensing information
**
****************************************************************************/

#ifndef TST_QMIMEDATABASE_H_INCLUDED
#define TST_QMIMEDATABASE_H_INCLUDED

#include <QtCore/QObject>

class tst_qmimedatabase : public QObject
{
    Q_OBJECT

public:
    tst_qmimedatabase();
    ~tst_qmimedatabase();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_mimeTypeForName();
    void test_mimeTypeForFileName_data();
    void test_mimeTypeForFileName();
    void test_findMultipleByName_data();
    void test_findMultipleByName();
    void test_inheritance();
    void test_aliases();
    void test_icons();
    void test_findByFileWithContent();
    void test_findByUrl();
    void test_findByContent_data();
    void test_findByContent();
    void test_mimeTypeForFileAndContent_data();
    void test_mimeTypeForFileAndContent();
    void test_allMimeTypes();
    void test_inheritsPerformance();
    void test_suffixes_data();
    void test_suffixes();
    void test_knownSuffix();
    void test_fromThreads();

    // shared-mime-info test suite

    void findByFileName_data();
    void findByFileName();

    void findByData_data();
    void findByData();

    void findByFile_data();
    void findByFile();

    //

    void installNewGlobalMimeType();
    void installNewLocalMimeType();

private:
    QString m_dataHome;
};

#endif   // TST_QMIMEDATABASE_H_INCLUDED
