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

#include <algorithm>
#include <functional>

#include "magicmatcher_p.h"
#include "mimetypeparser_p.h"
#include "qmimetype_p.h"

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QMimeDatabasePrivate, staticMimeDataBase)

QMimeDatabasePrivate::QMimeDatabasePrivate() :
    maxLevel(-1)
{
    // Assign here to avoid non-local static data initialization issues.
//    kModifiedMimeTypesPath = ICore::instance()->userResourcePath() + QLatin1String("/mimetypes/");
#warning TODO: FIX!!!
}

QMimeDatabasePrivate::~QMimeDatabasePrivate()
{
    qDeleteAll(typeMimeTypeMap);
}

bool QMimeDatabasePrivate::addMimeTypes(QIODevice *device, const QString &fileName, QString *errorMessage)
{
    if (errorMessage)
        errorMessage->clear();

    MimeTypeParser parser(*this);
    return parser.parse(device, fileName, errorMessage);
}

bool QMimeDatabasePrivate::addMimeTypes(const QString &fileName, QString *errorMessage)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage)
            *errorMessage = QString::fromLatin1("Cannot open %1: %2").arg(fileName, file.errorString());
        return false;
    }

    if (errorMessage)
        errorMessage->clear();

    return addMimeTypes(&file, fileName, errorMessage);
}

bool QMimeDatabasePrivate::addMimeTypes(QIODevice *device, QString *errorMessage)
{
    if (errorMessage)
        errorMessage->clear();

    return addMimeTypes(device, QLatin1String("<stream>"), errorMessage);
}

bool QMimeDatabasePrivate::addMimeType(QMimeType mt)
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

    maxLevel = -1; // Mark as dirty

    return true;
}

void QMimeDatabasePrivate::raiseLevelRecursion(MimeMapEntry &e, int level)
{
    if (e.level == MimeMapEntry::Dangling || e.level < level)
        e.level = level;

    if (maxLevel < level)
        maxLevel = level;

    // At all events recurse over children since nodes might have been added;
    // look them up in the type->MIME type map
    foreach (const QString &alias, parentChildrenMap.values(e.type.type())) {
        MimeMapEntry *entry = typeMimeTypeMap.value(resolveAlias(alias));
        if (!entry) {
            qWarning("%s: Inconsistent MIME hierarchy detected, child %s of %s cannot be found.",
                     Q_FUNC_INFO, alias.toLocal8Bit().constData(), e.type.type().toLocal8Bit().constData());
        } else {
            raiseLevelRecursion(*entry, level + 1);
        }
    }
}

void QMimeDatabasePrivate::determineLevels()
{
    // Loop over toplevels and recurse down their hierarchies.
    // Determine top levels by subtracting the children from the parent
    // set. Note that a toplevel at this point might have 'subclassesOf'
    // set to some class that is not in the DB, so, checking for an empty
    // 'subclassesOf' set is not sufficient to find the toplevels.
    // First, take the parent->child entries  whose parent exists and build
    // sets of parents/children
    QSet<QString> parentSet, childrenSet;
    ParentChildrenMap::const_iterator pit = parentChildrenMap.constBegin();
    for ( ; pit != parentChildrenMap.constEnd(); ++pit) {
        if (typeMimeTypeMap.contains(pit.key())) {
            parentSet.insert(pit.key());
            childrenSet.insert(pit.value());
        }
    }

    foreach (const QString &topLevel, parentSet.subtract(childrenSet)) {
        MimeMapEntry *entry = typeMimeTypeMap.value(resolveAlias(topLevel));
        if (!entry) {
            qWarning("%s: Inconsistent MIME hierarchy detected, top level element %s cannot be found.",
                     Q_FUNC_INFO, topLevel.toLocal8Bit().constData());
        } else {
            raiseLevelRecursion(*entry, 0);
        }
    }

    // move all danglings to top level
    foreach (MimeMapEntry *entry, typeMimeTypeMap) {
        if (entry->level == MimeMapEntry::Dangling)
            entry->level = 0;
    }
}

QMimeType QMimeDatabasePrivate::findByType(const QString &typeOrAlias) const
{
    const MimeMapEntry *entry = typeMimeTypeMap.value(resolveAlias(typeOrAlias));
    if (entry)
        return entry->type;
    return QMimeType();
}

unsigned QMimeDatabasePrivate::matchesBySuffix(const QMimeType &type, const QString &name, unsigned *length) const
{
    foreach (const QMimeGlobPattern &gp, type.d->globPatterns) {
        if (gp.regExp().exactMatch(name)) {
            *length = gp.regExp().pattern().length();
            return gp.weight();
        }
    }
    return 0;
}

QMimeType QMimeDatabasePrivate::findByName(const QString &name, unsigned *priorityPtr) const
{
    // Is the hierarchy set up in case we find several matches?
    if (maxLevel < 0) {
        QMimeDatabasePrivate *db = const_cast<QMimeDatabasePrivate *>(this);
        db->determineLevels();
    }

    QMimeType candidate;
    unsigned length = 0;

    for (int level = maxLevel; level >= 0 /*&& !candidate.isValid()*/; --level) {
        foreach (const MimeMapEntry *entry, typeMimeTypeMap) {
            if (entry->level == level) {
                unsigned currentLength;
                const unsigned suffixPriority = matchesBySuffix(entry->type, name, &currentLength);
                if (suffixPriority && (suffixPriority > *priorityPtr
                                       || (suffixPriority == *priorityPtr && currentLength > length))) {
                    length = currentLength;
                    *priorityPtr = suffixPriority;
                    candidate = entry->type;
                }
            }
        }
    }

    return candidate;
}

QMimeType QMimeDatabasePrivate::findByData(const QByteArray &data, unsigned *priorityPtr) const
{
    // Is the hierarchy set up in case we find several matches?
    if (maxLevel < 0) {
        QMimeDatabasePrivate *db = const_cast<QMimeDatabasePrivate *>(this);
        db->determineLevels();
    }

    QMimeType candidate;

    for (int level = maxLevel; level >= 0; --level) {
        foreach (const MimeMapEntry *entry, typeMimeTypeMap) {
            if (entry->level == level) {
                const unsigned contentPriority = entry->type.d->matchesData(data);
                if (contentPriority && contentPriority > *priorityPtr) {
                    *priorityPtr = contentPriority;
                    candidate = entry->type;
                }
            }
        }
    }

        return candidate;
}

QMimeType QMimeDatabasePrivate::findByFile(const QFileInfo &f, unsigned *priorityPtr) const
{
    // First, glob patterns are evaluated. If there is a match with max weight,
    // this one is selected and we are done. Otherwise, the file contents are
    // evaluated and the match with the highest value (either a magic priority or
    // a glob pattern weight) is selected. Matching starts from max level (most
    // specific) in both cases, even when there is already a suffix matching candidate.
    *priorityPtr = 0;
    FileMatchContext context(f);

    // Pass 1) Try to match on suffix#type
    QMimeType candidateByName = findByName(f.fileName(), priorityPtr);

    // Pass 2) Match on content
    if (!f.isReadable())
        return candidateByName;

    if (candidateByName.matchesData(context.data()) > MIN_MATCH_WEIGHT)
        return candidateByName;

    unsigned priorityByName = *priorityPtr;
    QMimeType candidateByData(findByData(context.data(), priorityPtr));

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

QList<QMimeMagicRuleMatcher > QMimeDatabasePrivate::magicMatchers() const
{
    QList<QMimeMagicRuleMatcher> magicMatchers;

    foreach (const MimeMapEntry *entry, typeMimeTypeMap)
        magicMatchers.append(entry->type.magicMatchers());

    return magicMatchers;
}

QList<QMimeType> QMimeDatabasePrivate::mimeTypes() const
{
    QList<QMimeType> mimeTypes;

    foreach (const MimeMapEntry *entry, typeMimeTypeMap)
        mimeTypes.append(entry->type);

    return mimeTypes;
}


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
    \o A Map QString->MimeMapEntry for the types (MimeMapEntry being a pair of
       QMimeType and (hierarchy) level.
    \o A map  QString->QString representing parent->child relations (enabling
       recursing over children)
    \o Using strings avoids dangling pointers.
    \endlist

    The hierarchy level is used for mapping by file types. When findByFile()
    is first called after addMimeType() it recurses over the hierarchy and sets
    the hierarchy level of the entries accordingly (0 toplevel, 1 first
    order...). It then does several passes over the type map, checking the
    globs for maxLevel, maxLevel-1....until it finds a match (idea being to
    to check the most specific types first). Starting a recursion from the
    leaves is not suitable since it will hit parent nodes several times.

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

bool QMimeDatabase::addMimeType(const QMimeType &mt)
{
    QMutexLocker locker(&d->mutex);

    return d->addMimeType(mt);
}

bool QMimeDatabase::addMimeTypes(const QString &fileName, QString *errorMessage)
{
    QMutexLocker locker(&d->mutex);

    return d->addMimeTypes(fileName, errorMessage);
}

bool QMimeDatabase::addMimeTypes(QIODevice *device, QString *errorMessage)
{
    QMutexLocker locker(&d->mutex);

    return d->addMimeTypes(device, errorMessage);
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

    unsigned priority = 0;
    return d->findByFile(fileInfo, &priority);
}

/*!
    Returns a MIME type for \a name or Null one if none found.
    This function does not try to open the file. To determine the MIME type by it's content, use
    QMimeDatabase::findByFile instead.
*/
QMimeType QMimeDatabase::findByName(const QString &name) const
{
    QMutexLocker locker(&d->mutex);

    unsigned priority = 0;
    return d->findByName(QFileInfo(name).fileName(), &priority);
}

/*!
    Returns a MIME type for \a data or Null one if none found. This function reads content of a file
    and tries to determine it's type using magic sequencies.
*/
QMimeType QMimeDatabase::findByData(const QByteArray &data) const
{
    QMutexLocker locker(&d->mutex);

    unsigned priority = 0;
    return d->findByData(data, &priority);
}

QList<QMimeType> QMimeDatabase::mimeTypes() const
{
    QMutexLocker locker(&d->mutex);

    return d->mimeTypes();
}

QList<QMimeMagicRuleMatcher> QMimeDatabase::magicMatchers() const
{
    QMutexLocker locker(&d->mutex);

    return d->magicMatchers();
}

/*!
    Returns all known suffixes
*/
//QStringList QMimeDatabase::suffixes() const
//{
    //QMutexLocker locker(&d->mutex);
//
    //return d->suffixes();
//}

QString QMimeDatabase::preferredSuffixByType(const QString &type) const
{
    QMutexLocker locker(&d->mutex);

    const QMimeType mt = d->findByType(type);

    return mt.isValid() ? mt.preferredSuffix() : QString();
}

QString QMimeDatabase::preferredSuffixByFile(const QFileInfo &f) const
{
    QMutexLocker locker(&d->mutex);

    unsigned priority = 0;
    const QMimeType mt = d->findByFile(f, &priority);

    return mt.isValid() ? mt.preferredSuffix() : QString();
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
