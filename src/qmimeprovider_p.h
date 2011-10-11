#ifndef QMIMEPROVIDER_P_H
#define QMIMEPROVIDER_P_H

#include "qmimedatabase_p.h"
class QFile;

class QMimeProviderBase
{
public:
    QMimeProviderBase(QMimeDatabasePrivate *db);
    virtual ~QMimeProviderBase() {}

    virtual bool isValid() = 0;
    virtual void ensureTypesLoaded() = 0;
    virtual QStringList findByName(const QString &fileName, QString *foundSuffix) = 0;
    virtual void ensureMagicLoaded() = 0;

    QMimeDatabasePrivate* m_db;
};

/*
   Parses mime.cache on demand
 */
class QMimeBinaryProvider : public QMimeProviderBase
{
public:
    QMimeBinaryProvider(QMimeDatabasePrivate *db);
    virtual ~QMimeBinaryProvider();

    virtual bool isValid();
    virtual void ensureTypesLoaded();
    virtual QStringList findByName(const QString &fileName, QString *foundSuffix);
    virtual void ensureMagicLoaded();

private:
    struct CacheFile;
    struct GlobMatchResult
    {
        QStringList matchingMimeTypes;
        int weight;
        int matchingPatternLength;
        QString foundSuffix;
    };

    void matchGlobList(GlobMatchResult& result, CacheFile *cacheFile, int offset, const QString &fileName);
    void matchSuffixTree(GlobMatchResult& result, CacheFile *cacheFile, int numEntries, int firstOffset, const QString &fileName);

    QList<CacheFile *> m_cacheFiles;
};

/*
   Parses the raw XML files (slower)
 */
class QMimeXMLProvider : public QMimeProviderBase
{
public:
    QMimeXMLProvider(QMimeDatabasePrivate *db);

    virtual bool isValid();
    virtual void ensureTypesLoaded();
    virtual QStringList findByName(const QString &fileName, QString *foundSuffix);
    virtual void ensureMagicLoaded();

    // Called by the mimetype xml parser
    bool addMimeType(const QMimeType &mt);
    void addGlobPattern(const QMimeGlobPattern& glob);

private:
    virtual void ensureLoaded();
    virtual void load(const QString &fileName);
    bool m_loaded;

    QMimeAllGlobPatterns m_mimeTypeGlobs;
};

#endif // QMIMEPROVIDER_P_H
