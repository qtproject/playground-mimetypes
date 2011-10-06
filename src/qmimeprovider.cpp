#include "qmimeprovider_p.h"
#include "mimetypeparser_p.h"
#include <qstandardpaths.h>
#include <QFile>
#include <QDebug>

QMimeProviderBase::QMimeProviderBase(QMimeDatabasePrivate *db)
    : m_db(db)
{
}

QMimeBinaryProvider::QMimeBinaryProvider(QMimeDatabasePrivate *db)
    : QMimeProviderBase(db)
{
}

bool QMimeBinaryProvider::isValid()
{
    const QString globalCacheFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("mime/mime.cache"));
    if (!globalCacheFile.isEmpty()) {
        const QString localCacheFile = QStandardPaths::storageLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/mime/mime.cache");
        qDebug() << globalCacheFile << localCacheFile;
        return globalCacheFile != localCacheFile;
    }
    return false;
}

void QMimeBinaryProvider::ensureTypesLoaded()
{
}

void QMimeBinaryProvider::ensureGlobsLoaded()
{
}

void QMimeBinaryProvider::ensureMagicLoaded()
{
}

QMimeXMLProvider::QMimeXMLProvider(QMimeDatabasePrivate *db)
    : QMimeProviderBase(db), m_loaded(false)
{
}

bool QMimeXMLProvider::isValid()
{
    return true;
}

void QMimeXMLProvider::ensureTypesLoaded()
{
    ensureLoaded();
}

void QMimeXMLProvider::ensureGlobsLoaded()
{
    ensureLoaded();
}

void QMimeXMLProvider::ensureMagicLoaded()
{
    ensureLoaded();
}

void QMimeXMLProvider::ensureLoaded()
{
    if (!m_loaded) {
        m_loaded = true;

        // TODO  look for freedesktop.org.xml file in the system
        //       if not found, use Qt's own copy

        // TODO: putting the xml file in the resource is a hack for now
        // We should instead install the file as part of installing Qt

        // TODO and then we also need to locate user-modified mimetypes, using QStandardPaths::storageLocation() + QDir::entryList
        const QString fileName = QLatin1String(":/qmime/freedesktop.org.xml");
        load(fileName);
    }
}

void QMimeXMLProvider::load(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("QMimeDatabase: Error loading %s\n%s", qPrintable(fileName), qPrintable(file.errorString()));
        return;
    }

    QString errorMessage;
    MimeTypeParser parser(*m_db);
    if (!parser.parse(&file, fileName, &errorMessage))
        qWarning("QMimeDatabase: Error loading %s\n%s", qPrintable(fileName), qPrintable(errorMessage));
}

