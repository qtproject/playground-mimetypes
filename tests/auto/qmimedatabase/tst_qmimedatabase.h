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
    void test_mimeTypesForFileName_data();
    void test_mimeTypesForFileName();
    void test_inheritance();
    void test_aliases();
    void test_icons();
    void test_mimeTypeForFileWithContent();
    void test_mimeTypeForUrl();
    void test_mimeTypeForData_data();
    void test_mimeTypeForData();
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
