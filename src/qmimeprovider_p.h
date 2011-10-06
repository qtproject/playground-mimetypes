#ifndef QMIMEPROVIDER_P_H
#define QMIMEPROVIDER_P_H

#include "qmimedatabase_p.h"

class QMimeProviderBase
{
public:
    QMimeProviderBase(QMimeDatabasePrivate *db);

    virtual bool isValid() = 0;
    virtual void ensureTypesLoaded() = 0;
    virtual void ensureGlobsLoaded() = 0;
    virtual void ensureMagicLoaded() = 0;

    QMimeDatabasePrivate* m_db;
};

/*
   Parses mime.cache (possibly on demand, for its different areas)
 */
class QMimeBinaryProvider : public QMimeProviderBase
{
public:
    QMimeBinaryProvider(QMimeDatabasePrivate *db);

    virtual bool isValid();
    virtual void ensureTypesLoaded();
    virtual void ensureGlobsLoaded();
    virtual void ensureMagicLoaded();

private:
    const QStringList m_cacheFiles;
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
    virtual void ensureGlobsLoaded();
    virtual void ensureMagicLoaded();

private:
    virtual void ensureLoaded();
    virtual void load(const QString &fileName);
    bool m_loaded;
};

#endif // QMIMEPROVIDER_P_H
