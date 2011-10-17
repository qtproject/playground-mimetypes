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
#include <QtCore/QDebug>
#include <qplatformdefs.h>

#include <algorithm>
#include <functional>

#include "magicmatcher_p.h"
#include "qmimeprovider_p.h"
#include "qmimetype_p.h"

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QMimeDatabasePrivate, staticMimeDataBase)

QMimeDatabasePrivate::QMimeDatabasePrivate()
    : m_provider(0)
{
}

QMimeDatabasePrivate::~QMimeDatabasePrivate()
{
    qDeleteAll(nameMimeTypeMap);
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

bool QMimeDatabasePrivate::addMimeType(const QMimeType &mt)
{
    if (!mt.isValid())
        return false;

    const QString &name = mt.name();

    // insert the MIME type.
    nameMimeTypeMap.insert(name, new MimeTypeMapEntry(mt));

#if 0 // This parentChildrenMap seems to be unused?
    // Register the children, resolved via alias map. Note that it is still
    // possible that aliases end up in the map if the parent classes are not inserted
    // at this point (thus their aliases not known).
    foreach (const QString &subClassOf, mt.subClassOf())
        parentChildrenMap.insert(resolveAlias(subClassOf), name);
#endif

    // register aliasses
    foreach (const QString &alias, mt.aliases())
        aliasMap.insert(alias, name);

    return true;
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

// Returns a MIME type or Null one if none found
QMimeType QMimeDatabasePrivate::mimeTypeForName(const QString &nameOrAlias)
{
    provider()->ensureTypesLoaded();

    const MimeTypeMapEntry *entry = nameMimeTypeMap.value(resolveAlias(nameOrAlias));
    if (entry)
        return entry->type;
    return QMimeType();
}

QStringList QMimeDatabasePrivate::findByName(const QString &fileName)
{
    QString foundSuffix;

    const QStringList matchingMimeTypes = provider()->findByName(QFileInfo(fileName).fileName(), &foundSuffix);
    // TODO a method that returns the found suffix

    return matchingMimeTypes;
}

// Returns a MIME type or Null one if none found
QMimeType QMimeDatabasePrivate::findByData(const QByteArray &data, unsigned *priorityPtr)
{
    provider()->ensureMagicLoaded();

    QMimeType candidate;

    // TODO delegate to backend, implement properly.
    foreach (const MimeTypeMapEntry *entry, nameMimeTypeMap) {
        const unsigned contentPriority = entry->type.d->matchesData(data);
        if (contentPriority && contentPriority > *priorityPtr) {
            *priorityPtr = contentPriority;
            candidate = entry->type;
        }
    }

    return candidate;
}

#if 0
// Return all known suffixes
QStringList QMimeDatabasePrivate::suffixes() const
{
    QStringList rc;
    const NameMimeTypeMap::const_iterator cend = nameMimeTypeMap.constEnd();
    for (NameMimeTypeMap::const_iterator it = nameMimeTypeMap.constBegin(); it != cend; ++it)
        rc += it.value()->type.suffixes();
    return rc;
}

QList<QMimeGlobPattern> QMimeDatabasePrivate::globPatterns() const
{
    QList<QMimeGlobPattern> globPatterns;
    const NameMimeTypeMap::const_iterator cend = nameMimeTypeMap.constEnd();
    for (NameMimeTypeMap::const_iterator it = nameMimeTypeMap.constBegin(); it != cend; ++it)
        globPatterns.append(toGlobPatterns(it.value()->type.globPatterns(), it.value()->type.type()));
    return globPatterns;
}

void QMimeDatabasePrivate::setGlobPatterns(const QString &nameOrAlias,
                                           const QStringList &globPatterns)
{
    NameMimeTypeMap::iterator tit =  nameMimeTypeMap.find(resolveAlias(nameOrAlias));
    if (tit != nameMimeTypeMap.end()) {
        QMimeTypeData mimeTypeData = QMimeTypeData(tit.value()->type);
        mimeTypeData.globPatterns = globPatterns;
        tit.value()->type = QMimeType(mimeTypeData);
    }
}

void QMimeDatabasePrivate::setMagicMatchers(const QString &nameOrAlias,
                                            const QList<QMimeMagicRuleMatcher> &matchers)
{
    NameMimeTypeMap::iterator tit = nameMimeTypeMap.find(resolveAlias(nameOrAlias));
    if (tit != nameMimeTypeMap.end()) {
        QMimeTypeData mimeTypeData = QMimeTypeData(tit.value()->type);
        mimeTypeData.magicMatchers = matchers;
        tit.value()->type = QMimeType(mimeTypeData);
    }
}
#endif

// Returns a MIME type or Null one if none found
QMimeType QMimeDatabasePrivate::findByNameAndData(const QString &fileName, QIODevice *device, unsigned *accuracyPtr)
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

        unsigned magicAccuracy = 0;
        QMimeType candidateByData(findByData(context.data(), &magicAccuracy));

        // Disambiguate conflicting extensions (if magic found something and the magicrule was < 80)
        if (candidateByData.isValid() && magicAccuracy > 0) {
            // "for glob_match in glob_matches:"
            // "if glob_match is subclass or equal to sniffed_type, use glob_match"
            const QString sniffedMime = candidateByData.name();
            foreach(const QString &m, candidatesByName) {
                // TODO for speed: provider->inherits(a,b) without loading QMimeType(a).
                QMimeType mimeFromPattern = mimeTypeForName(m);
                if (mimeFromPattern.inherits(sniffedMime)) {
                    // We have magic + pattern pointing to this, so it's a pretty good match
                    *accuracyPtr = 100;
                    return mimeFromPattern;
                }
            }
        }
    }

    if (candidatesByName.count() > 1) {
        *accuracyPtr = 20;
        candidatesByName.sort(); // to make it deterministic
        const QMimeType mime = mimeTypeForName(candidatesByName.at(0));
        if (mime.isValid())
            return mime;
    }

    // ### TODO: should be application/octet-stream, but the docu says Null
    return QMimeType();
}

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

QList<QMimeType> QMimeDatabasePrivate::mimeTypes() const
{
    QList<QMimeType> result;

    foreach (const MimeTypeMapEntry *entry, nameMimeTypeMap)
        result.append(entry->type);

    return result;
}

#if 0
QList<QMimeGlobPattern> QMimeDatabasePrivate::toGlobPatterns(const QStringList &patterns, const QString &mimeType, int weight)
{
    QList<QMimeGlobPattern> globPatterns;
    foreach (const QString &pattern, patterns) {
        globPatterns.append(QMimeGlobPattern(pattern, mimeType, weight, Qt::CaseSensitive));
    }
    return globPatterns;
}

QStringList QMimeDatabasePrivate::fromGlobPatterns(const QList<QMimeGlobPattern> &globPatterns)
{
    QStringList patterns;
    foreach (const QMimeGlobPattern &globPattern, globPatterns)
        patterns.append(globPattern.pattern());
    return patterns;
}
#endif


// TODO rewrite docu, it explains implementation details
/*!
    \class QMimeDatabase
    \brief MIME database to which the plugins can add the MIME types they handle.

    The class is protected by a QMutex and can therefore be accessed by threads.

    A good testcase is to run it over \c '/usr/share/mime/<*>/<*>.xml' on Linux.

    When adding a "text/plain" to it, the mimetype will receive a magic matcher
    that checks for text files that do not match the globs by heuristics.

    \section1 Design Considerations

    Storage requirements:
    \list
    \o Must be robust in case of incomplete hierarchies, dangling entries
    \o Plugins will not load and register their MIME types in order of inheritance.
    \o Multiple inheritance (several subClassOf) can occur
    \o Provide quick lookup by name
    \o Provide quick lookup by file type.
    \endlist

    This basically rules out some pointer-based tree, so the structure chosen is:
    \list
    \o An alias map QString->QString for mapping aliases to types
    \o A map  QString->QString representing parent->child relations (enabling
       recursing over children)
    \o Using strings avoids dangling pointers.
    \endlist

    \sa QMimeType, QMimeMagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa BaseMimeTypeParser, MimeTypeParser
*/

QMimeDatabase::QMimeDatabase() :
    d(staticMimeDataBase())
{
}

#if 0
QMimeDatabase::QMimeDatabase(QMimeDatabasePrivate *const theD) :
    d(theD)
{
}
#endif

QMimeDatabase::~QMimeDatabase()
{
#if 0
    if (d != staticMimeDataBase()) {
        delete d;
    }
#endif

    d = 0;
}

#if 0
QMimeDatabaseBuilder::QMimeDatabaseBuilder(QMimeDatabase *mimeDatabase) :
    d(mimeDatabase->data_ptr())
{
}

QMimeDatabaseBuilder::~QMimeDatabaseBuilder()
{
}

bool QMimeDatabaseBuilder::addMimeType(const QMimeType &mt)
{
    QMutexLocker locker(&d->mutex);

    return d->addMimeType(mt);
}
#endif

/*!
    Returns a MIME type for \a nameOrAlias or Null one if none found.
*/
QMimeType QMimeDatabase::mimeTypeForName(const QString &nameOrAlias) const
{
    QMutexLocker locker(&d->mutex);

    return d->mimeTypeForName(nameOrAlias);
}

/*!
    Returns a MIME type for \a fileInfo or Null one if none found.

    If \a fileInfo is a unix symbolic link, the file that it refers to
    will be used instead.
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

    unsigned priority = 0;
    return d->findByNameAndData(fileInfo.absoluteFilePath(), &file, &priority);
}

/*!
    Returns a MIME type for \a file or Null one if none found.
    The \a file can also include an absolute or relative path.
*/
QMimeType QMimeDatabase::findByFile(const QString &fileName) const
{
    //qDebug() << Q_FUNC_INFO << "fileName" << fileName;

    QMutexLocker locker(&d->mutex);

    QFile file(fileName);
    unsigned priority = 0;
    return d->findByNameAndData(fileName, &file, &priority);
}

/*!
    Returns a MIME type for the file \a name or Null one if none found.
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
        return QMimeType();
    else if (matchCount == 1)
        return d->mimeTypeForName(matches.first());
    else {
        // We have to pick one.
        matches.sort(); // Make it deterministic
        return d->mimeTypeForName(matches.first());
    }
}

/*!
    Returns a MIME type for \a data or Null one if none found.
*/
QMimeType QMimeDatabase::findByData(const QByteArray &data) const
{
    QMutexLocker locker(&d->mutex);

    unsigned int accuracy = 0;
    return d->findByData(data, &accuracy);
}

/*!
    Returns a MIME type for \a url or Null one if none found.
    If the url is a local file, this calls findByFile.
    Otherwise the matching is done based on the name only
    (except over schemes where filenames don't mean much, like HTTP)
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
    return findByName(url.path());
}

QMimeType QMimeDatabase::findByNameAndData(const QString &fileName, QIODevice *device) const
{
    //qDebug() << Q_FUNC_INFO << "fileName" << fileName;

    unsigned int accuracy = 0;
    return d->findByNameAndData(fileName, device, &accuracy);
}

QMimeType QMimeDatabase::findByNameAndData(const QString &fileName, const QByteArray &data) const
{
    //qDebug() << Q_FUNC_INFO << "fileName" << fileName;

    QBuffer buffer(const_cast<QByteArray *>(&data));
    unsigned int accuracy = 0;
    return d->findByNameAndData(fileName, &buffer, &accuracy);
}

#if 0
void QMimeDatabaseBuilder::setMagicMatchers(const QString &nameOrAlias,
                                            const QList<QMimeMagicRuleMatcher> &matchers)
{
    QMutexLocker locker(&d->mutex);

    d->setMagicMatchers(nameOrAlias, matchers);
}
#endif

QList<QMimeType> QMimeDatabase::mimeTypes() const
{
    QMutexLocker locker(&d->mutex);

    return d->mimeTypes();
}

#if 0
/*!
    Returns all known suffixes
*/
QStringList QMimeDatabaseBuilder::suffixes() const
{
    QMutexLocker locker(&d->mutex);

    return d->suffixes();
}

QString QMimeDatabaseBuilder::preferredSuffixByType(const QString &nameOrAlias) const
{
    d->mutex.lock();
    const QMimeType mt = d->mimeTypeForName(nameOrAlias);
    d->mutex.unlock();
    if (mt.isValid())
        return mt.preferredSuffix(); // already does Mutex locking
    return QString();
}

QString QMimeDatabaseBuilder::preferredSuffixByNameAndData(const QString &fileName, QIODevice *device) const
{
    d->mutex.lock();
    unsigned priority = 0;
    const QMimeType mt = d->findByNameAndData(fileName, device, &priority);
    d->mutex.unlock();
    if (mt.isValid())
        return mt.preferredSuffix(); // already does Mutex locking
    return QString();
}

bool QMimeDatabaseBuilder::setPreferredSuffix(const QString &nameOrAlias, const QString &suffix)
{
    QMutexLocker locker(&d->mutex);

    return d->setPreferredSuffix(nameOrAlias, suffix);
}

QList<QMimeGlobPattern> QMimeDatabaseBuilder::toGlobPatterns(const QStringList &patterns, const QString &mimeType, int weight)
{
    return QMimeDatabasePrivate::toGlobPatterns(patterns, mimeType, weight);
}

QStringList QMimeDatabaseBuilder::fromGlobPatterns(const QList<QMimeGlobPattern> &globPatterns)
{
    return QMimeDatabasePrivate::fromGlobPatterns(globPatterns);
}

QStringList QMimeDatabaseBuilder::globPatterns() const
{
    QMutexLocker locker(&d->mutex);

    return fromGlobPatterns(d->globPatterns());
}

void QMimeDatabaseBuilder::setGlobPatterns(const QString &nameOrAlias,
                                           const QStringList &globPatterns)
{
    QMutexLocker locker(&d->mutex);

    d->setGlobPatterns(nameOrAlias, globPatterns);
}
#endif

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

QT_END_NAMESPACE
