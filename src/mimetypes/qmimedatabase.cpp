/**************************************************************************
**
** This file is part of QMime
**
** Based on Qt Creator source code
**
** Qt Creator Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "qmimedatabase.h"
#include "qmimedatabase_p.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QSet>
#include <QtCore/QBuffer>
#include <QtCore/QUrl>
#include <QtCore/QStack>
#include <QtCore/QDebug>
#include <qplatformdefs.h>

#include <algorithm>
#include <functional>

#include "magicmatcher_p.h"
#include "qmimeprovider_p.h"
#include "qmimetype_p.h"

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QMimeDatabasePrivate, staticMimeDataBase)

QMimeDatabasePrivate *QMimeDatabasePrivate::instance()
{
    return staticMimeDataBase();
}

QMimeDatabasePrivate::QMimeDatabasePrivate()
    : m_provider(0), m_defaultMimeType(QLatin1String("application/octet-stream"))
{
}

QMimeDatabasePrivate::~QMimeDatabasePrivate()
{
    delete m_provider;
    m_provider = 0;
}

QMimeProviderBase *QMimeDatabasePrivate::provider()
{
    if (!m_provider) {
        QMimeProviderBase *binaryProvider = new QMimeBinaryProvider(this);
        if (binaryProvider->isValid())
            m_provider = binaryProvider;
        else {
            delete m_provider;
            m_provider = new QMimeXMLProvider(this);
        }
    }
    return m_provider;
}

void QMimeDatabasePrivate::setProvider(QMimeProviderBase *theProvider)
{
    delete m_provider;
    m_provider = theProvider;
}

#if 0
bool QMimeDatabasePrivate::setPreferredSuffix(const QString &nameOrAlias, const QString &suffix)
{
    NameMimeTypeMap::iterator tit =  nameMimeTypeMap.find(resolveAlias(nameOrAlias));
    if (tit != nameMimeTypeMap.end()) {
        QMimeTypeData mimeTypeData = QMimeTypeData(tit.value()->type);
        mimeTypeData.preferredSuffix = suffix;
        tit.value()->type = QMimeType(mimeTypeData);
        return true;
    }
    return false;
}
#endif

// Returns a MIME type or an invalid one if none found
QMimeType QMimeDatabasePrivate::mimeTypeForName(const QString &nameOrAlias)
{
    return provider()->mimeTypeForName(provider()->resolveAlias(nameOrAlias));
}

QStringList QMimeDatabasePrivate::findByName(const QString &fileName)
{
    QString foundSuffix;

    const QStringList matchingMimeTypes = provider()->findByName(QFileInfo(fileName).fileName(), &foundSuffix);
    // TODO a method that returns the found suffix

    return matchingMimeTypes;
}

QMimeType QMimeDatabasePrivate::findByData(const QByteArray &data, int *accuracyPtr)
{
    if (data.isEmpty()) {
        *accuracyPtr = 100;
        return mimeTypeForName(QLatin1String("application/x-zerosize"));
    }

    *accuracyPtr = 0;
    QMimeType candidate = provider()->findByMagic(data, accuracyPtr);

    if (candidate.isValid())
        return candidate;

    return mimeTypeForName(defaultMimeType());
}

QMimeType QMimeDatabasePrivate::findByNameAndData(const QString &fileName, QIODevice *device, int *accuracyPtr)
{
    // First, glob patterns are evaluated. If there is a match with max weight,
    // this one is selected and we are done. Otherwise, the file contents are
    // evaluated and the match with the highest value (either a magic priority or
    // a glob pattern weight) is selected. Matching starts from max level (most
    // specific) in both cases, even when there is already a suffix matching candidate.
    *accuracyPtr = 0;
    FileMatchContext context(device, fileName);

    // Pass 1) Try to match on the file name
    QStringList candidatesByName = findByName(fileName);

    if (candidatesByName.count() == 1) {
        *accuracyPtr = 100;
        const QMimeType mime = mimeTypeForName(candidatesByName.at(0));
        if (mime.isValid())
            return mime;
        candidatesByName.clear();
    }

    // Extension is unknown, or matches multiple mimetypes.
    // Pass 2) Match on content, if we can read the data
    if (context.isReadable()) {

        int magicAccuracy = 0;
        QMimeType candidateByData(findByData(context.data(), &magicAccuracy));

        // Disambiguate conflicting extensions (if magic found something and the magicrule was < 80)
        if (candidateByData.isValid() && magicAccuracy > 0) {
            // "for glob_match in glob_matches:"
            // "if glob_match is subclass or equal to sniffed_type, use glob_match"
            const QString sniffedMime = candidateByData.name();
            foreach(const QString &m, candidatesByName) {
                if (inherits(m, sniffedMime)) {
                    // We have magic + pattern pointing to this, so it's a pretty good match
                    *accuracyPtr = 100;
                    return mimeTypeForName(m);
                }
            }
            *accuracyPtr = magicAccuracy;
            return candidateByData;
        }
    }

    if (candidatesByName.count() > 1) {
        *accuracyPtr = 20;
        candidatesByName.sort(); // to make it deterministic
        const QMimeType mime = mimeTypeForName(candidatesByName.at(0));
        if (mime.isValid())
            return mime;
    }

    return mimeTypeForName(defaultMimeType());
}

#if 0
QStringList QMimeDatabasePrivate::filterStrings() const
{
    QStringList rc;

    foreach (const MimeTypeMapEntry *entry, nameMimeTypeMap) {
        const QString filterString = entry->type.filterString();
        if (!filterString.isEmpty())
            rc += filterString;
    }

    return rc;
}
#endif

QList<QMimeType> QMimeDatabasePrivate::allMimeTypes()
{
    return provider()->allMimeTypes();
}


bool QMimeDatabasePrivate::inherits(const QString &mime, const QString &parent)
{
    const QString resolvedParent = provider()->resolveAlias(parent);
    Q_ASSERT(provider()->resolveAlias(mime) == mime);
    QStack<QString> toCheck;
    toCheck.push(mime);
    while (!toCheck.isEmpty()) {
        const QString current = toCheck.pop();
        if (current == resolvedParent)
            return true;
        foreach(const QString &par, provider()->parents(current)) {
            toCheck.push(par);
        }
    }
    return false;
}

// TODO write more docu
/*!
    \class QMimeDatabase
    \brief Database of MIME types

    The class is protected by a QMutex and can therefore be accessed by threads.

    \sa QMimeType
*/

QMimeDatabase::QMimeDatabase() :
    d(staticMimeDataBase())
{
}

QMimeDatabase::~QMimeDatabase()
{
    d = 0;
}

/*!
    Returns a MIME type for \a nameOrAlias or an invalid one if none found.
*/
QMimeType QMimeDatabase::mimeTypeForName(const QString& nameOrAlias) const
{
    QMutexLocker locker(&d->mutex);

    return d->mimeTypeForName(nameOrAlias);
}

/*!
    Returns a MIME type for \a fileInfo.

    This method looks at both the file name and the file contents,
    if necessary. The file extension has priority over the contents,
    but the contents will be used if the file extension is unknown, or
    matches multiple MIME types.

    A valid MIME type is always returned. If the file doesn't match any
    known extension or data, the default MIME type (application/octet-stream)
    is returned.

    If \a fileInfo is a unix symbolic link, the file that it refers to
    will be used instead.

    \see isDefault
*/
QMimeType QMimeDatabase::findByFile(const QFileInfo &fileInfo) const
{
    //qDebug() << Q_FUNC_INFO << "fileInfo" << fileInfo.absoluteFilePath();

    QMutexLocker locker(&d->mutex);

    if (fileInfo.isDir())
        return d->mimeTypeForName(QLatin1String("inode/directory"));

    QFile file(fileInfo.absoluteFilePath());

#ifdef Q_OS_UNIX
    // Cannot access statBuf.st_mode from the filesystem engine, so we have to stat again.
    const QByteArray nativeFilePath = QFile::encodeName(file.fileName());
    QT_STATBUF statBuffer;
    if (QT_LSTAT(nativeFilePath, &statBuffer) == 0) {
        if (S_ISCHR(statBuffer.st_mode))
            return d->mimeTypeForName(QLatin1String("inode/chardevice"));
        if (S_ISBLK(statBuffer.st_mode))
            return d->mimeTypeForName(QLatin1String("inode/blockdevice"));
        if (S_ISFIFO(statBuffer.st_mode))
            return d->mimeTypeForName(QLatin1String("inode/fifo"));
        if (S_ISSOCK(statBuffer.st_mode))
            return d->mimeTypeForName(QLatin1String("inode/socket"));
    }
#endif

    int priority = 0;
    return d->findByNameAndData(fileInfo.absoluteFilePath(), &file, &priority);
}

/*!
    Returns a MIME type for \a file.

    This method looks at both the file name and the file contents,
    if necessary. The file extension has priority over the contents,
    but the contents will be used if the file extension is unknown, or
    matches multiple MIME types.

    A valid MIME type is always returned. If the file doesn't match any
    known pattern or data, the default MIME type (application/octet-stream)
    is returned.

    The \a file can also include an absolute or relative path.
*/
QMimeType QMimeDatabase::findByFile(const QString &fileName) const
{
    // Implemented as a wrapper around findByFile(QFileInfo), so no mutex.
    QFileInfo fileInfo(fileName);
    return findByFile(fileInfo);
}

/*!
    Returns a MIME type for the file \a fileName.

    A valid MIME type is always returned. If the file name doesn't match any
    known pattern, the default MIME type (application/octet-stream)
    is returned.

    This function does not try to open the file. To also use the content
    when determining the MIME type, use QMimeDatabase::findByFile or
    QMimeDatabase::findByNameAndData instead.
*/
QMimeType QMimeDatabase::findByName(const QString &fileName) const
{
    QMutexLocker locker(&d->mutex);

    QStringList matches = d->findByName(fileName);
    const int matchCount = matches.count();
    if (matchCount == 0)
        return d->mimeTypeForName(d->defaultMimeType());
    else if (matchCount == 1)
        return d->mimeTypeForName(matches.first());
    else {
        // We have to pick one.
        matches.sort(); // Make it deterministic
        return d->mimeTypeForName(matches.first());
    }
}

/*!
    Returns a MIME type for \a data.

    A valid MIME type is always returned. If \a data doesn't match any
    known MIME type data, the default MIME type (application/octet-stream)
    is returned.
*/
QMimeType QMimeDatabase::findByData(const QByteArray &data) const
{
    QMutexLocker locker(&d->mutex);

    int accuracy = 0;
    return d->findByData(data, &accuracy);
}

/*!
    Returns a MIME type for \a url.

    If the url is a local file, this calls findByFile.
    Otherwise the matching is done based on the name only
    (except over schemes where filenames don't mean much, like HTTP)

    A valid MIME type is always returned. If \a data doesn't match any
    known MIME type data, the default MIME type (application/octet-stream)
    is returned.
*/
QMimeType QMimeDatabase::findByUrl(const QUrl &url) const
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (url.isLocalFile())
        return findByFile(url.toLocalFile());
#else
    QString localFile(url.toLocalFile());
    if (!localFile.isEmpty())
        return findByFile(localFile);
#endif

    const QString scheme = url.scheme();
    if (scheme.startsWith(QLatin1String("http")))
        return mimeTypeForName(d->defaultMimeType());

    return findByName(url.path());
}

/*!
    Returns a MIME type for the given \a fileName and \a device data.

    This overload can be useful when the file is remote, and we started to
    download some of its data in a device. This allows to do full MIME type
    matching for remote files as well.

    A valid MIME type is always returned. If \a data doesn't match any
    known MIME type data, the default MIME type (application/octet-stream)
    is returned.

    This method looks at both the file name and the file contents,
    if necessary. The file extension has priority over the contents,
    but the contents will be used if the file extension is unknown, or
    matches multiple MIME types.
*/
QMimeType QMimeDatabase::findByNameAndData(const QString &fileName, QIODevice *device) const
{
    //qDebug() << Q_FUNC_INFO << "fileName" << fileName;

    int accuracy = 0;
    return d->findByNameAndData(fileName, device, &accuracy);
}

/*!
    Returns a MIME type for the given \a fileName and \a device data.

    This overload can be useful when the file is remote, and we started to
    download some of its data. This allows to do full MIME type matching for
    remote files as well.

    A valid MIME type is always returned. If \a data doesn't match any
    known MIME type data, the default MIME type (application/octet-stream)
    is returned.

    This method looks at both the file name and the file contents,
    if necessary. The file extension has priority over the contents,
    but the contents will be used if the file extension is unknown, or
    matches multiple MIME types.
*/
QMimeType QMimeDatabase::findByNameAndData(const QString &fileName, const QByteArray &data) const
{
    //qDebug() << Q_FUNC_INFO << "fileName" << fileName;

    QBuffer buffer(const_cast<QByteArray *>(&data));
    int accuracy = 0;
    return d->findByNameAndData(fileName, &buffer, &accuracy);
}

/*!
    Returns the list of all available MIME types.

    This can be useful for showing all MIME types to the user, for instance
    in a MIME type editor. Do not use unless really necessary in other cases
    though, prefer using the findBy* methods for performance reasons.
*/
QList<QMimeType> QMimeDatabase::allMimeTypes() const
{
    QMutexLocker locker(&d->mutex);

    return d->allMimeTypes();
}

// TODO: needed?
#if 0
QStringList QMimeDatabase::filterStrings() const
{
    QMutexLocker locker(&d->mutex);

    return d->filterStrings();
}

/*!
    Returns a string with all the possible file filters, for use with file dialogs
*/
QString QMimeDatabase::allFiltersString(QString *allFilesFilter) const
{
    if (allFilesFilter)
        allFilesFilter->clear();

    // Compile list of filter strings, sort, and remove duplicates (different MIME types might
    // generate the same filter).
    QStringList filters = filterStrings();
    if (filters.empty())
        return QString();
    filters.sort();
    filters.erase(std::unique(filters.begin(), filters.end()), filters.end());

    static const QString allFiles = QObject::tr("All Files (*)", "QMimeDatabase");
    if (allFilesFilter)
        *allFilesFilter = allFiles;

    // Prepend all files filter (instead of appending to work around a bug in Qt/Mac).
    filters.prepend(allFiles);

    return filters.join(QLatin1String(";;"));
}
#endif

QT_END_NAMESPACE
