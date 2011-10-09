#include "qmimeprovider_p.h"
#include "mimetypeparser_p.h"
#include <qstandardpaths.h>
#include <QFile>
#include <QDir>
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
    m_cacheFiles = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QLatin1String("mime/mime.cache"));
    return false; // HACK FOR NOW

    if (m_cacheFiles.count() > 1)
        return true;
    if (m_cacheFiles.isEmpty())
        return false;

    // We found exactly one file; is it the user-modified mimes, or a system file?
    const QString foundFile = m_cacheFiles.first();
    const QString localCacheFile = QStandardPaths::storageLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/mime/mime.cache");

    return foundFile != localCacheFile;
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

        bool fdoXmlFound = false;
        QStringList allFiles;

        const QStringList packageDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QLatin1String("mime/packages"), QStandardPaths::LocateDirectory);
        foreach (const QString &packageDir, packageDirs) {
            QDir dir(packageDir);
            const QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
            qDebug() << packageDir << files;
            if (!fdoXmlFound)
                fdoXmlFound = files.contains(QLatin1String("freedesktop.org.xml"));
            foreach (const QString& file, files) {
                allFiles.append(packageDir + QLatin1Char('/') + file);
            }
        }

        if (!fdoXmlFound) {
            // TODO: putting the xml file in the resource is a hack for now
            // We should instead install the file as part of installing Qt
            load(QLatin1String(":/qmime/freedesktop.org.xml"));
        }

        foreach (const QString& file, allFiles)
            load(file);
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

