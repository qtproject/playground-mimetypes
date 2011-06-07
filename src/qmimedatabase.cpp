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

const QString QMimeDatabasePrivate::kModifiedMimeTypesFile(QLatin1String("modifiedmimetypes.xml"));
QString QMimeDatabasePrivate::kModifiedMimeTypesPath;

QMimeDatabasePrivate::QMimeDatabasePrivate()
    : maxLevel(-1)
{
    // Assign here to avoid non-local static data initialization issues.
//    kModifiedMimeTypesPath = ICore::instance()->userResourcePath() + QLatin1String("/mimetypes/");
#warning TODO: FIX!!!
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
    // look them up in the type->mime type map
    foreach (const QString &alias, parentChildrenMap.values(e.type.type())) {
        MimeMapEntry *entry = typeMimeTypeMap.value(resolveAlias(alias));
        if (!entry) {
            qWarning("%s: Inconsistent mime hierarchy detected, child %s of %s cannot be found.",
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
            qWarning("%s: Inconsistent mime hierarchy detected, top level element %s cannot be found.",
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

bool QMimeDatabasePrivate::setPreferredSuffix(const QString &typeOrAlias, const QString &suffix)
{
    MimeMapEntry *entry = typeMimeTypeMap.value(resolveAlias(typeOrAlias));
    if (entry)
        return entry->type.setPreferredSuffix(suffix);
    return false;
}

QMimeType QMimeDatabasePrivate::findByType(const QString &typeOrAlias) const
{
    const MimeMapEntry *entry = typeMimeTypeMap.value(resolveAlias(typeOrAlias));
    if (entry)
        return entry->type;
    return QMimeType();
}

static inline unsigned matchesBySuffix(const QMimeType &type, const QString &name, unsigned *length)
{
    foreach (const QMimeGlobPattern &gp, type.globPatterns()) {
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

static inline bool isTextFile(const QByteArray &data)
{
    // UTF16 byte order marks
    static const char bigEndianBOM[] = "\xFE\xFF";
    static const char littleEndianBOM[] = "\xFF\xFE";

    const char *p = data.constData();
    const char *e = p + data.size();
    for ( ; p < e; ++p) {
        if (*p >= 0x01 && *p < 0x09) // Sure-fire binary
            return false;

        if (*p == 0) // Check for UTF16
            return data.startsWith(bigEndianBOM) || data.startsWith(littleEndianBOM);
    }

    return true;
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

    if (candidate.isValid())
        return candidate;

    // Hack
    // TODO: use low fallback priorities (2 and 1)?
    if (isTextFile(data))
        candidate = findByType(QLatin1String("text/plain")); // try to guess if it is text

    if (candidate.isValid())
        return candidate;

    return findByType(QLatin1String("application/octet-stream")); // fallback to application/octet-stream
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

// Return all known suffixes
QStringList QMimeDatabasePrivate::suffixes() const
{
    QStringList rc;

    foreach (const MimeMapEntry *entry, typeMimeTypeMap)
        rc += entry->type.suffixes();

    return rc;
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

QList<QMimeGlobPattern> QMimeDatabasePrivate::globPatterns() const
{
    QList<QMimeGlobPattern> globPatterns;

    foreach (const MimeMapEntry *entry, typeMimeTypeMap)
        globPatterns.append(entry->type.globPatterns());

    return globPatterns;
}

void QMimeDatabasePrivate::setGlobPatterns(const QString &typeOrAlias,
                                          const QList<QMimeGlobPattern> &globPatterns)
{
    MimeMapEntry *entry = typeMimeTypeMap.value(resolveAlias(typeOrAlias));
    if (entry)
        entry->type.setGlobPatterns(globPatterns);
}

QList<QMimeMagicRuleMatcher > QMimeDatabasePrivate::magicMatchers() const
{
    QList<QMimeMagicRuleMatcher> magicMatchers;

    foreach (const MimeMapEntry *entry, typeMimeTypeMap)
        magicMatchers.append(entry->type.magicMatchers());

    return magicMatchers;
}

void QMimeDatabasePrivate::setMagicMatchers(const QString &typeOrAlias,
                                            const QList<QMimeMagicRuleMatcher> &matchers)
{
    MimeMapEntry *entry = typeMimeTypeMap.value(resolveAlias(typeOrAlias));
    if (entry)
        entry->type.setMagicMatchers(matchers);
}

QList<QMimeType> QMimeDatabasePrivate::mimeTypes() const
{
    QList<QMimeType> mimeTypes;

    foreach (const MimeMapEntry *entry, typeMimeTypeMap)
        mimeTypes.append(entry->type);

    return mimeTypes;
}

void QMimeDatabasePrivate::syncUserModifiedMimeTypes()
{
    QHash<QString, QMimeType> userModified;

    foreach (const QMimeType &userMimeType, QMimeDatabasePrivate::readUserModifiedMimeTypes())
        userModified.insert(userMimeType.type(), userMimeType);

    foreach (MimeMapEntry *entry, typeMimeTypeMap) {
        QHash<QString, QMimeType>::const_iterator userMimeIt = userModified.constFind(entry->type.type());
        if (userMimeIt != userModified.constEnd()) {
            entry->type.setGlobPatterns(userMimeIt.value().globPatterns());
            entry->type.setMagicMatchers(userMimeIt.value().magicMatchers());
        }
    }
}

void QMimeDatabasePrivate::clearUserModifiedMimeTypes()
{
    // This removes the user's file. However, the operation will actually take place the next time
    // Creator starts, since we currently don't support removing stuff from the mime database.
    QFile::remove(kModifiedMimeTypesPath + kModifiedMimeTypesFile);
}

QList<QMimeGlobPattern> QMimeDatabasePrivate::toGlobPatterns(const QStringList &patterns, int weight)
{
    QList<QMimeGlobPattern> globPatterns;

    foreach (const QString &pattern, patterns) {
        const QRegExp wildcard(pattern, Qt::CaseSensitive, QRegExp::WildcardUnix);
        globPatterns.append(QMimeGlobPattern(wildcard, weight));
    }

    return globPatterns;
}

QStringList QMimeDatabasePrivate::fromGlobPatterns(const QList<QMimeGlobPattern> &globPatterns)
{
    QStringList patterns;

    foreach (const QMimeGlobPattern &globPattern, globPatterns)
        patterns.append(globPattern.regExp().pattern());

    return patterns;
}


/*!
    \class MimeDatabase
    \brief Mime data base to which the plugins can add the mime types they handle.

    The class is protected by a QMutex and can therefore be accessed by threads.

    A good testcase is to run it over \c '/usr/share/mime/<*>/<*>.xml' on Linux.

    When adding a "text/plain" to it, the mimetype will receive a magic matcher
    that checks for text files that do not match the globs by heuristics.

    \section1 Design Considerations

    Storage requirements:
    \list
    \o Must be robust in case of incomplete hierarchies, dangling entries
    \o Plugins will not load and register their mime types in order of inheritance.
    \o Multiple inheritance (several subClassesOf) can occur
    \o Provide quick lookup by name
    \o Provide quick lookup by file type.
    \endlist

    This basically rules out some pointer-based tree, so the structure chosen is:
    \list
    \o An alias map QString->QString for mapping aliases to types
    \o A Map QString->MimeMapEntry for the types (MimeMapEntry being a pair of
       MimeType and (hierarchy) level.
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

    \sa MimeType, MagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa BinaryMatcher, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
*/

QMimeDatabase::QMimeDatabase()
    : d(new QMimeDatabasePrivate)
{
}

QMimeDatabase::~QMimeDatabase()
{
    qDeleteAll(d->typeMimeTypeMap);
    delete d;
}

/*!
    Returns a mime type for \a typeOrAlias or Null one if none found.
*/
QMimeType QMimeDatabase::findByType(const QString &typeOrAlias) const
{
    QMutexLocker locker(&d->mutex);

    return d->findByType(typeOrAlias);
}

/*!
    Returns a mime type for \a fileInfo or Null one if none found.
*/
QMimeType QMimeDatabase::findByFile(const QFileInfo &fileInfo) const
{
    QMutexLocker locker(&d->mutex);

    unsigned priority = 0;
    return d->findByFile(fileInfo, &priority);
}

/*!
    Returns a mime type for \a name or Null one if none found.
    This function does not tries to open file to determine mime type by it's content, use
    QMimeDatabase::findByFile instead.
*/
QMimeType QMimeDatabase::findByName(const QString &name) const
{
    QMutexLocker locker(&d->mutex);

    unsigned priority = 0;
    return d->findByName(QFileInfo(name).fileName(), &priority);
}

/*!
    Returns a mime type for \a data or Null one if none found. This function reads content of a file
    and tries to determine it's type using magic sequencies.
*/
QMimeType QMimeDatabase::findByData(const QByteArray &data) const
{
    QMutexLocker locker(&d->mutex);

    unsigned priority = 0;
    return d->findByData(data, &priority);
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

QStringList QMimeDatabase::suffixes() const
{
    QMutexLocker locker(&d->mutex);

    return d->suffixes();
}

QStringList QMimeDatabase::filterStrings() const
{
    QMutexLocker locker(&d->mutex);

    return d->filterStrings();
}

QString QMimeDatabase::allFiltersString(QString *allFilesFilter) const
{
    if (allFilesFilter)
        allFilesFilter->clear();

    // Compile list of filter strings, sort, and remove duplicates (different mime types might
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

QList<QMimeGlobPattern> QMimeDatabase::globPatterns() const
{
    QMutexLocker locker(&d->mutex);

    return d->globPatterns();
}

void QMimeDatabase::setGlobPatterns(const QString &typeOrAlias,
                                   const QList<QMimeGlobPattern> &globPatterns)
{
    QMutexLocker locker(&d->mutex);

    d->setGlobPatterns(typeOrAlias, globPatterns);
}

QList<QMimeMagicRuleMatcher> QMimeDatabase::magicMatchers() const
{
    QMutexLocker locker(&d->mutex);

    return d->magicMatchers();
}

void QMimeDatabase::setMagicMatchers(const QString &typeOrAlias,
                                     const QList<QMimeMagicRuleMatcher> &matchers)
{
    QMutexLocker locker(&d->mutex);

    d->setMagicMatchers(typeOrAlias, matchers);
}

QList<QMimeType> QMimeDatabase::mimeTypes() const
{
    QMutexLocker locker(&d->mutex);

    return d->mimeTypes();
}

void QMimeDatabase::syncUserModifiedMimeTypes()
{
    QMutexLocker locker(&d->mutex);

    d->syncUserModifiedMimeTypes();
}

void QMimeDatabase::clearUserModifiedMimeTypes()
{
    QMutexLocker locker(&d->mutex);

    d->clearUserModifiedMimeTypes();
}

QList<QMimeType> QMimeDatabase::readUserModifiedMimeTypes()
{
    return QMimeDatabasePrivate::readUserModifiedMimeTypes();
}

void QMimeDatabase::writeUserModifiedMimeTypes(const QList<QMimeType> &mimeTypes)
{
    QMimeDatabasePrivate::writeUserModifiedMimeTypes(mimeTypes);
}

QString QMimeDatabase::preferredSuffixByType(const QString &type) const
{
    const QMimeType mt = findByType(type);
    if (mt.isValid())
        return mt.preferredSuffix(); // already does Mutex locking
    return QString();
}

QString QMimeDatabase::preferredSuffixByFile(const QFileInfo &f) const
{
    const QMimeType mt = findByFile(f);
    if (mt.isValid())
        return mt.preferredSuffix(); // already does Mutex locking
    return QString();
}

bool QMimeDatabase::setPreferredSuffix(const QString &typeOrAlias, const QString &suffix)
{
    QMutexLocker locker(&d->mutex);

    return d->setPreferredSuffix(typeOrAlias, suffix);
}

QList<QMimeGlobPattern> QMimeDatabase::toGlobPatterns(const QStringList &patterns, int weight)
{
    return QMimeDatabasePrivate::toGlobPatterns(patterns, weight);
}

QStringList QMimeDatabase::fromGlobPatterns(const QList<QMimeGlobPattern> &globPatterns)
{
    return QMimeDatabasePrivate::fromGlobPatterns(globPatterns);
}

QT_END_NAMESPACE
