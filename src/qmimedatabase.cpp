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

#include <algorithm>
#include <functional>

#include "qmimeprovider_p.h"
#include "magicmatcher_p.h"
#include "qmimetype_p.h"

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QMimeDatabasePrivate, staticMimeDataBase)

QMimeDatabasePrivate::QMimeDatabasePrivate()
{
}

QMimeDatabasePrivate::~QMimeDatabasePrivate()
{
    qDeleteAll(typeMimeTypeMap);
}

QMimeProviderBase * QMimeDatabasePrivate::provider()
{
    if (!m_provider) {
        QMimeProviderBase *provider = new QMimeBinaryProvider(this);
        if (provider->isValid())
            m_provider = provider;
        else {
            delete m_provider;
            m_provider = new QMimeXMLProvider(this);
        }
    }
    return m_provider;
}

void QMimeDatabasePrivate::addGlobPattern(const QMimeGlobPattern& glob)
{
    m_mimeTypeGlobs.addGlob(glob);
}

bool QMimeDatabasePrivate::addMimeType(const QMimeType &mt)
{
    if (!mt.isValid())
        return false;

    const QString &type = mt.type();

    // insert the type.
    typeMimeTypeMap.insert(type, new MimeMapEntry(mt));

    // Register the children, resolved via alias map. Note that it is still
    // possible that aliases end up in the map if the parent classes are not inserted
    // at this point (thus their aliases not known).
    foreach (const QString &subClassOf, mt.subClassOf())
        parentChildrenMap.insert(resolveAlias(subClassOf), type);

    // register aliasses
    foreach (const QString &alias, mt.aliases())
        aliasMap.insert(alias, type);

    return true;
}

QMimeType QMimeDatabasePrivate::findByType(const QString &typeOrAlias)
{
    provider()->ensureTypesLoaded();
    const MimeMapEntry *entry = typeMimeTypeMap.value(resolveAlias(typeOrAlias));
    if (entry)
        return entry->type;
    return QMimeType();
}

QStringList QMimeDatabasePrivate::findByName(const QString &fileName)
{
    provider()->ensureGlobsLoaded();

    QString foundExt;
    const QStringList matchingMimeTypes = m_mimeTypeGlobs.matchingGlobs(fileName, &foundExt);

    //if (pMatchingExtension)
    //    *pMatchingExtension = foundExt;
    return matchingMimeTypes;
}

QMimeType QMimeDatabasePrivate::findByData(const QByteArray &data, unsigned *priorityPtr)
{
    provider()->ensureMagicLoaded();
    QMimeType candidate;

    foreach (const MimeMapEntry *entry, typeMimeTypeMap) {
        const unsigned contentPriority = entry->type.d->matchesData(data);
        if (contentPriority && contentPriority > *priorityPtr) {
            *priorityPtr = contentPriority;
            candidate = entry->type;
        }
    }

    return candidate;
}

QMimeType QMimeDatabasePrivate::findByNameAndData(const QString &fileName, QIODevice *device, unsigned *priorityPtr)
{
    // First, glob patterns are evaluated. If there is a match with max weight,
    // this one is selected and we are done. Otherwise, the file contents are
    // evaluated and the match with the highest value (either a magic priority or
    // a glob pattern weight) is selected. Matching starts from max level (most
    // specific) in both cases, even when there is already a suffix matching candidate.
    *priorityPtr = 0;
    FileMatchContext context(device, fileName);

    // Pass 1) Try to match on suffix#type
    QStringList candidatesByName = findByName(fileName);

    // TODO REWRITE THIS METHOD, FOR PROPER GLOB-CONFLICT HANDLING

    QMimeType candidateByName;
    if (!candidatesByName.isEmpty()) {
        *priorityPtr = 50;
        candidateByName = findByType(candidatesByName.last());
    }

    // Pass 2) Match on content
    if (!context.isReadable())
        return candidateByName;

    if (candidateByName.matchesData(context.data()) > 50 /* TODO REWRITE ALL THIS */)
        return candidateByName;

    unsigned priorityByName = *priorityPtr;
    QMimeType candidateByData(findByData(context.data(), priorityPtr));

    // ## BROKEN, PRIORITIES HAVE A DIFFERENT SCALE
    return priorityByName < *priorityPtr ? candidateByData : candidateByName;
}

QStringList QMimeDatabasePrivate::filterStrings() const
{
    QStringList rc;

    foreach (const MimeMapEntry *entry, typeMimeTypeMap) {
        const QString filterString = entry->type.filterString();
        if (!filterString.isEmpty())
            rc += filterString;
    }

    return rc;
}

QList<QMimeType> QMimeDatabasePrivate::mimeTypes() const
{
    QList<QMimeType> mimeTypes;

    foreach (const MimeMapEntry *entry, typeMimeTypeMap)
        mimeTypes.append(entry->type);

    return mimeTypes;
}


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
    \o Multiple inheritance (several subClassesOf) can occur
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
    \sa BinaryMatcher, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
*/

QMimeDatabase::QMimeDatabase() :
    d(staticMimeDataBase())
{
}

QMimeDatabase::~QMimeDatabase()
{
}

/*!
    Returns a MIME type for \a typeOrAlias or Null one if none found.
*/
QMimeType QMimeDatabase::findByType(const QString &typeOrAlias) const
{
    QMutexLocker locker(&d->mutex);

    return d->findByType(typeOrAlias);
}

/*!
    Returns a MIME type for \a fileInfo or Null one if none found.
*/
QMimeType QMimeDatabase::findByFile(const QFileInfo &fileInfo) const
{
    QMutexLocker locker(&d->mutex);

    QFile file(fileInfo.absoluteFilePath());
    unsigned priority = 0;
    return d->findByNameAndData(fileInfo.fileName(), &file, &priority);
}

/*!
    Returns a MIME type for \a file or Null one if none found.
    The \a file can also include an absolute or relative path.
*/
QMimeType QMimeDatabase::findByFile(const QString &fileName) const
{
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
QMimeType QMimeDatabase::findByName(const QString &name) const
{
    QMutexLocker locker(&d->mutex);

    QStringList matches = d->findByName(QFileInfo(name).fileName());
    const int matchCount = matches.count();
    if (matchCount == 0)
        return QMimeType();
    else if (matchCount == 1)
        return d->findByType(matches.first());
    else {
        // We have to pick one.
        matches.sort(); // Make it deterministic
        return d->findByType(matches.first());
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
    if (url.isLocalFile())
        return findByFile(url.toLocalFile());
    return findByName(url.path());
}

QMimeType QMimeDatabase::findByNameAndData(const QString &name, QIODevice *device) const
{
    unsigned int accuracy = 0;
    return d->findByNameAndData(name, device, &accuracy);
}

QMimeType QMimeDatabase::findByNameAndData(const QString &name, const QByteArray &data) const
{
    QBuffer buffer(const_cast<QByteArray *>(&data));
    unsigned int accuracy = 0;
    return d->findByNameAndData(name, &buffer, &accuracy);
}

QList<QMimeType> QMimeDatabase::mimeTypes() const
{
    QMutexLocker locker(&d->mutex);

    return d->mimeTypes();
}

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
