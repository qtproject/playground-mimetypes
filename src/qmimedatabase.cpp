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
#include "qmimetype_p.h"
#include "magicmatcher_p.h"
#include "qmimedatabase_p.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

#include <algorithm>
#include <functional>

const QChar QMimeDatabasePrivate::kSemiColon(QLatin1Char(';'));
const QString QMimeDatabasePrivate::kModifiedMimeTypesFile(QLatin1String("modifiedmimetypes.xml"));
QString QMimeDatabasePrivate::kModifiedMimeTypesPath;

QMimeDatabasePrivate::QMimeDatabasePrivate() :
    m_maxLevel(-1)
{
    // Assign here to avoid non-local static data initialization issues.
//    kModifiedMimeTypesPath = ICore::instance()->userResourcePath() + QLatin1String("/mimetypes/");
#warning TODO: FIX!!!
}

/*!
    \class MimeTypeParser
    \brief Mime type parser

    Populates MimeDataBase

    \sa MimeDatabase, IMagicMatcher, MagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa FileMatchContext, BinaryMatcher, HeuristicTextMagicMatcher
    \sa MimeTypeParser
*/

bool QMimeDatabasePrivate::addMimeTypes(QIODevice *device, const QString &fileName, QString *errorMessage)
{
    MimeTypeParser parser(*this);
    return parser.parse(device, fileName, errorMessage);
}

bool QMimeDatabasePrivate::addMimeTypes(const QString &fileName, QString *errorMessage)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        *errorMessage = QString::fromLatin1("Cannot open %1: %2").arg(fileName, file.errorString());
        return false;
    }
    return addMimeTypes(&file, fileName, errorMessage);
}

bool QMimeDatabasePrivate::addMimeTypes(QIODevice *device, QString *errorMessage)
{
    return addMimeTypes(device, QLatin1String("<stream>"), errorMessage);
}

bool QMimeDatabasePrivate::addMimeType(QMimeType mt)
{
    if (!mt.isValid())
        return false;

    const QString type = mt.type();
    // Hack: Add a magic text matcher to "text/plain" and the fallback matcher to
    // binary types "application/octet-stream"
    if (type == QLatin1String(textTypeC)) {
        mt.addMagicMatcher(QSharedPointer<IMagicMatcher>(new HeuristicTextMagicMatcher));
    } else {
        if (type == QLatin1String(binaryTypeC))
            mt.addMagicMatcher(QSharedPointer<IMagicMatcher>(new BinaryMatcher));
    }

    // insert the type.
    m_typeMimeTypeMap.insert(type, MimeMapEntry(mt));

    // Register the children, resolved via alias map. Note that it is still
    // possible that aliases end up in the map if the parent classes are not inserted
    // at this point (thus their aliases not known).
    const QStringList subClassesOf = mt.subClassOf();
    for (int i = 0; i < subClassesOf.size(); i++)
        m_parentChildrenMap.insert(resolveAlias(subClassesOf.at(i)), type);

    // register aliasses
    const QStringList aliases = mt.aliases();
    for (int i = 0; i < aliases.size(); i++)
        m_aliasMap.insert(aliases.at(i), type);

    m_maxLevel = -1; // Mark as dirty
    return true;
}

QString QMimeDatabasePrivate::resolveAlias(const QString &name) const
{
    return m_aliasMap.value(name, name);
}

void QMimeDatabasePrivate::raiseLevelRecursion(MimeMapEntry &e, int level)
{
    if (e.level == Dangling || e.level < level)
        e.level = level;

    if (m_maxLevel < level)
        m_maxLevel = level;
    // At all events recurse over children since nodes might have been
    // added.
    const QStringList childTypes = m_parentChildrenMap.values(e.type.type());
    if (childTypes.empty())
        return;
    // look them up in the type->mime type map
    const int nextLevel = level + 1;
    const TypeMimeTypeMap::iterator tm_end = m_typeMimeTypeMap.end();
    for (int i = 0; i < childTypes.size(); i++) {
        const QString &alias = childTypes.at(i);
        const TypeMimeTypeMap::iterator tm_it = m_typeMimeTypeMap.find(resolveAlias(alias));
        if (tm_it == tm_end) {
            qWarning("%s: Inconsistent mime hierarchy detected, child %s of %s cannot be found.",
                     Q_FUNC_INFO, alias.toUtf8().constData(), e.type.type().toUtf8().constData());
        } else {
            raiseLevelRecursion(*tm_it, nextLevel);
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
    const ParentChildrenMap::const_iterator pcend = m_parentChildrenMap.constEnd();
    for (ParentChildrenMap::const_iterator it =  m_parentChildrenMap.constBegin(); it !=  pcend; ++it)
        if (m_typeMimeTypeMap.contains(it.key())) {
            parentSet.insert(it.key());
            childrenSet.insert(it.value());
        }
    const QSet<QString> topLevels = parentSet.subtract(childrenSet);
    if (debugMimeDB)
        qDebug() << Q_FUNC_INFO << "top levels" << topLevels;
    const TypeMimeTypeMap::iterator tm_end = m_typeMimeTypeMap.end();
    const QSet<QString>::const_iterator tl_cend = topLevels.constEnd();
    for (QSet<QString>::const_iterator tl_it =  topLevels.constBegin(); tl_it !=  tl_cend; ++tl_it) {
        const TypeMimeTypeMap::iterator tm_it = m_typeMimeTypeMap.find(resolveAlias(*tl_it));
        if (tm_it == tm_end) {
            qWarning("%s: Inconsistent mime hierarchy detected, top level element %s cannot be found.",
                     Q_FUNC_INFO, tl_it->toUtf8().constData());
        } else {
            raiseLevelRecursion(tm_it.value(), 0);
        }
    }

    // move all danglings to top level
    TypeMimeTypeMap::iterator cend = m_typeMimeTypeMap.end();
    for (TypeMimeTypeMap::iterator it = m_typeMimeTypeMap.begin(); it != cend; ++it) {
        if (it.value().level == Dangling) {
            it.value().level = 0;
        }
    }
}

bool QMimeDatabasePrivate::setPreferredSuffix(const QString &typeOrAlias, const QString &suffix)
{
    TypeMimeTypeMap::iterator tit = m_typeMimeTypeMap.find(resolveAlias(typeOrAlias));
    if (tit != m_typeMimeTypeMap.end())
        return tit.value().type.setPreferredSuffix(suffix);
    return false;
}

QMimeType QMimeDatabasePrivate::findByType(const QString &typeOrAlias) const
{
    return m_typeMimeTypeMap.value(resolveAlias(typeOrAlias)).type;
}

// Debugging wrapper around findByFile()
QMimeType QMimeDatabasePrivate::findByFile(const QFileInfo &f) const
{
    unsigned priority = 0;
    if (debugMimeDB)
        qDebug() << '>' << Q_FUNC_INFO << f.absoluteFilePath();

    if (f.isDir())
        return findByType("inode/directory");

    const QMimeType rc = findByFile(f, &priority);
    if (debugMimeDB) {
        if (rc.isValid()) {
            qDebug() << "<MimeDatabase::findByFile: match prio=" << priority << rc.type();
        } else {
            qDebug() << "<MimeDatabase::findByFile: no match";
        }
    }
    return rc;
}

QMimeType QMimeDatabasePrivate::findByName(const QString &name) const
{
    // Is the hierarchy set up in case we find several matches?
    if (m_maxLevel < 0) {
        QMimeDatabasePrivate *db = const_cast<QMimeDatabasePrivate *>(this);
        db->determineLevels();
    }

    QMimeType candidate;

    unsigned priority = 0;

    const TypeMimeTypeMap::const_iterator cend = m_typeMimeTypeMap.constEnd();
    for (int level = m_maxLevel; level >= 0 && !candidate.isValid(); level--) {
        for (TypeMimeTypeMap::const_iterator it = m_typeMimeTypeMap.constBegin(); it != cend; ++it) {
            if (it.value().level == level) {
                const unsigned suffixPriority = it.value().type.m_d->matchesFileBySuffix(name);
                if (suffixPriority && suffixPriority > priority) {
                    priority = suffixPriority;
                    candidate = it.value().type;
                    if (suffixPriority >= MimeGlobPattern::MaxWeight)
                        return candidate;
                }
            }
        }
    }

    return candidate;
}

QMimeType QMimeDatabasePrivate::findByFile(const QFileInfo &f, unsigned *priorityPtr) const
{
    // Is the hierarchy set up in case we find several matches?
    if (m_maxLevel < 0) {
        QMimeDatabasePrivate *db = const_cast<QMimeDatabasePrivate *>(this);
        db->determineLevels();
    }

    // First, glob patterns are evaluated. If there is a match with max weight,
    // this one is selected and we are done. Otherwise, the file contents are
    // evaluated and the match with the highest value (either a magic priority or
    // a glob pattern weight) is selected. Matching starts from max level (most
    // specific) in both cases, even when there is already a suffix matching candidate.
    *priorityPtr = 0;
    QMimeType candidate;
    FileMatchContext context(f);

    // Pass 1) Try to match on suffix#type
    const TypeMimeTypeMap::const_iterator cend = m_typeMimeTypeMap.constEnd();
    for (int level = m_maxLevel; level >= 0 && !candidate.isValid(); level--) {
        for (TypeMimeTypeMap::const_iterator it = m_typeMimeTypeMap.constBegin(); it != cend; ++it) {
            if (it.value().level == level) {
                const unsigned suffixPriority = it.value().type.m_d->matchesFileBySuffix(context);
                if (suffixPriority && suffixPriority > *priorityPtr) {
                    *priorityPtr = suffixPriority;
                    candidate = it.value().type;
                    if (suffixPriority >= MimeGlobPattern::MaxWeight)
                        return candidate;
                }
            }
        }
    }

    // Pass 2) Match on content
    if (!f.isReadable())
        return candidate;
    for (int level = m_maxLevel; level >= 0; level--) {
        for (TypeMimeTypeMap::const_iterator it = m_typeMimeTypeMap.constBegin(); it != cend; ++it) {
            if (it.value().level == level) {
                const unsigned contentPriority = it.value().type.m_d->matchesFileByContent(context);
                if (contentPriority && contentPriority > *priorityPtr) {
                    *priorityPtr = contentPriority;
                    candidate = it.value().type;
                }
            }
        }
    }

    return candidate;
}

// Debugging wrapper around findByData()
QMimeType QMimeDatabasePrivate::findByData(const QByteArray &data) const
{
    unsigned priority = 0;
    if (debugMimeDB)
        qDebug() << '>' << Q_FUNC_INFO << data.left(20).toHex();
    const QMimeType rc = findByData(data, &priority);
    if (debugMimeDB) {
        if (rc.isValid()) {
            qDebug() << "<MimeDatabase::findByData: match prio=" << priority << rc.type();
        } else {
            qDebug() << "<MimeDatabase::findByData: no match";
        }
    }
    return rc;
}

QMimeType QMimeDatabasePrivate::findByData(const QByteArray &data, unsigned *priorityPtr) const
{
    // Is the hierarchy set up in case we find several matches?
    if (m_maxLevel < 0) {
        QMimeDatabasePrivate *db = const_cast<QMimeDatabasePrivate *>(this);
        db->determineLevels();
    }

    *priorityPtr = 0;
    QMimeType candidate;

    const TypeMimeTypeMap::const_iterator cend = m_typeMimeTypeMap.constEnd();
    for (int level = m_maxLevel; level >= 0; level--)
        for (TypeMimeTypeMap::const_iterator it = m_typeMimeTypeMap.constBegin(); it != cend; ++it)
            if (it.value().level == level) {
                const unsigned contentPriority = it.value().type.m_d->matchesData(data);
                if (contentPriority && contentPriority > *priorityPtr) {
                    *priorityPtr = contentPriority;
                    candidate = it.value().type;
                }
            }

    return candidate;
}

// Return all known suffixes
QStringList QMimeDatabasePrivate::suffixes() const
{
    QStringList rc;

    const TypeMimeTypeMap::const_iterator cend = m_typeMimeTypeMap.constEnd();
    for (TypeMimeTypeMap::const_iterator it = m_typeMimeTypeMap.constBegin(); it != cend; ++it)
        rc += it.value().type.suffixes();

    return rc;
}

QStringList QMimeDatabasePrivate::filterStrings() const
{
    QStringList rc;

    const TypeMimeTypeMap::const_iterator cend = m_typeMimeTypeMap.constEnd();
    for (TypeMimeTypeMap::const_iterator it = m_typeMimeTypeMap.constBegin(); it != cend; ++it) {
        const QString filterString = it.value().type.filterString();
        if (!filterString.isEmpty())
            rc += filterString;
    }

    return rc;
}

QList<MimeGlobPattern> QMimeDatabasePrivate::globPatterns() const
{
    QList<MimeGlobPattern> globPatterns;

    const TypeMimeTypeMap::const_iterator cend = m_typeMimeTypeMap.constEnd();
    for (TypeMimeTypeMap::const_iterator it = m_typeMimeTypeMap.constBegin(); it != cend; ++it)
        globPatterns.append(it.value().type.globPatterns());

    return globPatterns;
}

void QMimeDatabasePrivate::setGlobPatterns(const QString &typeOrAlias,
                                          const QList<MimeGlobPattern> &globPatterns)
{
    TypeMimeTypeMap::iterator tit = m_typeMimeTypeMap.find(resolveAlias(typeOrAlias));
    if (tit != m_typeMimeTypeMap.end())
        tit.value().type.setGlobPatterns(globPatterns);
}

QList<QSharedPointer<IMagicMatcher> > QMimeDatabasePrivate::magicMatchers() const
{
    QList<QSharedPointer<IMagicMatcher> > magicMatchers;

    const TypeMimeTypeMap::const_iterator cend = m_typeMimeTypeMap.constEnd();
    for (TypeMimeTypeMap::const_iterator it = m_typeMimeTypeMap.constBegin(); it != cend; ++it)
        magicMatchers.append(it.value().type.magicMatchers());

    return magicMatchers;
}

void QMimeDatabasePrivate::setMagicMatchers(const QString &typeOrAlias,
                                           const QList<QSharedPointer<IMagicMatcher> > &matchers)
{
    TypeMimeTypeMap::iterator tit =  m_typeMimeTypeMap.find(resolveAlias(typeOrAlias));
    if (tit != m_typeMimeTypeMap.end())
        tit.value().type.setMagicMatchers(matchers);
}

QList<QMimeType> QMimeDatabasePrivate::mimeTypes() const
{
    QList<QMimeType> mimeTypes;

    const TypeMimeTypeMap::const_iterator cend = m_typeMimeTypeMap.constEnd();
    for (TypeMimeTypeMap::const_iterator it = m_typeMimeTypeMap.constBegin(); it != cend; ++it)
        mimeTypes.append(it.value().type);

    return mimeTypes;
}

void QMimeDatabasePrivate::syncUserModifiedMimeTypes()
{
    QHash<QString, QMimeType> userModified;
    const QList<QMimeType> &userMimeTypes = readUserModifiedMimeTypes();
    foreach (const QMimeType &userMimeType, userMimeTypes)
        userModified.insert(userMimeType.type(), userMimeType);

    TypeMimeTypeMap::iterator end = m_typeMimeTypeMap.end();
    QHash<QString, QMimeType>::const_iterator userMimeEnd = userModified.end();
    for (TypeMimeTypeMap::iterator it = m_typeMimeTypeMap.begin(); it != end; ++it) {
        QHash<QString, QMimeType>::const_iterator userMimeIt =
            userModified.find(it.value().type.type());
        if (userMimeIt != userMimeEnd) {
            it.value().type.setGlobPatterns(userMimeIt.value().globPatterns());
            it.value().type.setMagicRuleMatchers(userMimeIt.value().magicRuleMatchers());
        }
    }
}

QList<QMimeType> QMimeDatabasePrivate::readUserModifiedMimeTypes()
{
    QList<QMimeType> mimeTypes;
    QFile file(kModifiedMimeTypesPath + kModifiedMimeTypesFile);
    if (file.open(QFile::ReadOnly)) {
        QMimeType mimeType;
        QHash<int, QList<QMimeMagicRule> > rules;
        QXmlStreamReader reader(&file);
        QXmlStreamAttributes atts;
        while (!reader.atEnd()) {
            switch (reader.readNext()) {
            case QXmlStreamReader::StartElement:
                atts = reader.attributes();
                if (reader.name() == mimeTypeTagC) {
                    mimeType.setType(atts.value(mimeTypeAttributeC).toString());
                    const QString &patterns = atts.value(patternAttributeC).toString();
                    mimeType.setGlobPatterns(toGlobPatterns(patterns.split(kSemiColon)));
                } else if (reader.name() == matchTagC) {
                    const QString &value = atts.value(matchValueAttributeC).toString();
                    const QString &type = atts.value(matchTypeAttributeC).toString();
                    const QString &offset = atts.value(matchOffsetAttributeC).toString();
                    QPair<int, int> range = QMimeMagicRule::fromOffset(offset);
                    const int priority = atts.value(priorityAttributeC).toString().toInt();

                    QMimeMagicRule::Type magicType = QMimeMagicRule::stringToType(type.toLatin1());
                    if (magicType != QMimeMagicRule::Invalid)
                        rules[priority].append(QMimeMagicRule(magicType, value, range.first, range.second));
                }
                break;
            case QXmlStreamReader::EndElement:
                if (reader.name() == mimeTypeTagC) {
                    mimeType.setMagicRuleMatchers(MagicRuleMatcher::createMatchers(rules));
                    mimeTypes.append(mimeType);
                    mimeType.clear();
                    rules.clear();
                }
                break;
            default:
                break;
            }
        }
        if (reader.hasError())
            qWarning() << kModifiedMimeTypesFile << reader.errorString() << reader.lineNumber()
                       << reader.columnNumber();
        file.close();
    }
    return mimeTypes;
}

void QMimeDatabasePrivate::writeUserModifiedMimeTypes(const QList<QMimeType> &mimeTypes)
{
    // Keep mime types modified which are already on file, unless they are part of the current set.
    QSet<QString> currentMimeTypes;
    foreach (const QMimeType &mimeType, mimeTypes)
        currentMimeTypes.insert(mimeType.type());
    const QList<QMimeType> &inFileMimeTypes = readUserModifiedMimeTypes();
    QList<QMimeType> allModifiedMimeTypes = mimeTypes;
    foreach (const QMimeType &mimeType, inFileMimeTypes)
        if (!currentMimeTypes.contains(mimeType.type()))
            allModifiedMimeTypes.append(mimeType);

    if (QFile::exists(kModifiedMimeTypesPath) || QDir().mkpath(kModifiedMimeTypesPath)) {
        QFile file(kModifiedMimeTypesPath + kModifiedMimeTypesFile);
        if (file.open(QFile::WriteOnly | QFile::Truncate)) {
            // Notice this file only represents user modifications. It is writen in a
            // convienient way for synchronization, which is similar to but not exactly the
            // same format we use for the embedded mime type files.
            QXmlStreamWriter writer(&file);
            writer.setAutoFormatting(true);
            writer.writeStartDocument();
            writer.writeStartElement(QLatin1String(mimeInfoTagC));
            foreach (const QMimeType &mimeType, allModifiedMimeTypes) {
                writer.writeStartElement(mimeTypeTagC);
                writer.writeAttribute(mimeTypeAttributeC, mimeType.type());
                writer.writeAttribute(patternAttributeC,
                                      fromGlobPatterns(mimeType.globPatterns()).join(kSemiColon));
                const QList<QSharedPointer<IMagicMatcher> > &matchers = mimeType.magicMatchers();
                foreach (const QSharedPointer<IMagicMatcher> &matcher, matchers) {
                    // Only care about rule-based matchers.
                    if (MagicRuleMatcher *ruleMatcher =
                        dynamic_cast<MagicRuleMatcher *>(matcher.data())) {
                        const QList<QMimeMagicRule> &rules = ruleMatcher->magicRules();
                        foreach (const QMimeMagicRule &rule, rules) {
                            writer.writeStartElement(matchTagC);
                            writer.writeAttribute(matchValueAttributeC, rule.matchValue());
                            writer.writeAttribute(matchTypeAttributeC, rule.matchType());
                            writer.writeAttribute(matchOffsetAttributeC,
                                                  QMimeMagicRule::toOffset(
                                                      qMakePair(rule.startPos(), rule.endPos())));
                            writer.writeAttribute(priorityAttributeC,
                                                  QString::number(ruleMatcher->priority()));
                            writer.writeEndElement();
                        }
                    }
                }
                writer.writeEndElement();
            }
            writer.writeEndElement();
            writer.writeEndDocument();
            file.close();
        }
    }
}

void QMimeDatabasePrivate::clearUserModifiedMimeTypes()
{
    // This removes the user's file. However, the operation will actually take place the next time
    // Creator starts, since we currently don't support removing stuff from the mime database.
    QFile::remove(kModifiedMimeTypesPath + kModifiedMimeTypesFile);
}

QList<MimeGlobPattern> QMimeDatabasePrivate::toGlobPatterns(const QStringList &patterns, int weight)
{
    QList<MimeGlobPattern> globPatterns;

    foreach (const QString &pattern, patterns) {
        QRegExp regExp(pattern, Qt::CaseSensitive, QRegExp::Wildcard);
        globPatterns.append(MimeGlobPattern(regExp, weight));
    }

    return globPatterns;
}

QStringList QMimeDatabasePrivate::fromGlobPatterns(const QList<MimeGlobPattern> &globPatterns)
{
    QStringList patterns;

    foreach (const MimeGlobPattern &globPattern, globPatterns)
        patterns.append(globPattern.regExp().pattern());

    return patterns;
}

void QMimeDatabasePrivate::debug(QTextStream &str) const
{
    str << ">MimeDatabase\n";
    const TypeMimeTypeMap::const_iterator cend = m_typeMimeTypeMap.constEnd();
    for (TypeMimeTypeMap::const_iterator it = m_typeMimeTypeMap.constBegin(); it != cend; ++it) {
        str << "Entry level " << it.value().level << '\n';
        it.value().type.m_d->debug(str);
    }
    str << "<MimeDatabase\n";
}

QT_BEGIN_NAMESPACE

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

    \sa MimeType, IMagicMatcher, MagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa FileMatchContext, BinaryMatcher, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
*/

QMimeDatabase::QMimeDatabase() :
    m_d(new QMimeDatabasePrivate)
{
}

QMimeDatabase::~QMimeDatabase()
{
    delete m_d;
}

/*!
    \fn QMimeType QMimeDatabase::findByType(const QString &typeOrAlias) const

    Returns a mime type for \a typeOrAlias or Null one if none found.
*/
QMimeType QMimeDatabase::findByType(const QString &typeOrAlias) const
{
    m_d->m_mutex.lock();
    const QMimeType rc = m_d->findByType(typeOrAlias);
    m_d->m_mutex.unlock();
    return rc;
}

/*!
    \fn QMimeType QMimeDatabase::findByFile(const QFileInfo &fileInfo) const

    Returns a mime type for \a fileInfo or Null one if none found.
*/
QMimeType QMimeDatabase::findByFile(const QFileInfo &fileInfo) const
{
    m_d->m_mutex.lock();
    const QMimeType rc = m_d->findByFile(fileInfo);
    m_d->m_mutex.unlock();
    return rc;
}

/*!
    \fn QMimeType QMimeDatabase::findByName(const QString &name) const

    Returns a mime type for \a name or Null one if none found.
    This function does not tries to open file to determine mime type by it's content, use
    QMimeDatabase::findByFile instead.
*/
QMimeType QMimeDatabase::findByName(const QString &name) const
{
    m_d->m_mutex.lock();
    const QMimeType rc = m_d->findByName(name);
    m_d->m_mutex.unlock();
    return rc;
}

/*!
    \fn QMimeType QMimeDatabase::findByData(const QByteArray &data) const

    Returns a mime type for \a data or Null one if none found. This function reads content of a file
    and tries to determine it's type using magic sequencies.
*/
QMimeType QMimeDatabase::findByData(const QByteArray &data) const
{
    m_d->m_mutex.lock();
    const QMimeType rc = m_d->findByData(data);
    m_d->m_mutex.unlock();
    return rc;
}

bool QMimeDatabase::addMimeType(const QMimeType &mt)
{
    m_d->m_mutex.lock();
    const bool rc = m_d->addMimeType(mt);
    m_d->m_mutex.unlock();
    return rc;
}

bool QMimeDatabase::addMimeTypes(const QString &fileName, QString *errorMessage)
{
    m_d->m_mutex.lock();
    const bool rc = m_d->addMimeTypes(fileName, errorMessage);
    m_d->m_mutex.unlock();
    return rc;
}

bool QMimeDatabase::addMimeTypes(QIODevice *device, QString *errorMessage)
{
    m_d->m_mutex.lock();
    const bool rc = m_d->addMimeTypes(device, errorMessage);
    m_d->m_mutex.unlock();
    return rc;
}

QStringList QMimeDatabase::suffixes() const
{
    m_d->m_mutex.lock();
    const QStringList rc = m_d->suffixes();
    m_d->m_mutex.unlock();
    return rc;
}

QStringList QMimeDatabase::filterStrings() const
{
    m_d->m_mutex.lock();
    const QStringList rc = m_d->filterStrings();
    m_d->m_mutex.unlock();
    return rc;
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

QList<MimeGlobPattern> QMimeDatabase::globPatterns() const
{
    m_d->m_mutex.lock();
    const QList<MimeGlobPattern> rc = m_d->globPatterns();
    m_d->m_mutex.unlock();
    return rc;
}

void QMimeDatabase::setGlobPatterns(const QString &typeOrAlias,
                                   const QList<MimeGlobPattern> &globPatterns)
{
    m_d->m_mutex.lock();
    m_d->setGlobPatterns(typeOrAlias, globPatterns);
    m_d->m_mutex.unlock();
}

QMimeDatabase::IMagicMatcherList QMimeDatabase::magicMatchers() const
{
    m_d->m_mutex.lock();
    const IMagicMatcherList rc = m_d->magicMatchers();
    m_d->m_mutex.unlock();
    return rc;
}

void QMimeDatabase::setMagicMatchers(const QString &typeOrAlias,
                                    const IMagicMatcherList &matchers)
{
    m_d->m_mutex.lock();
    m_d->setMagicMatchers(typeOrAlias, matchers);
    m_d->m_mutex.unlock();
}

QList<QMimeType> QMimeDatabase::mimeTypes() const
{
    m_d->m_mutex.lock();
    const QList<QMimeType> &mimeTypes = m_d->mimeTypes();
    m_d->m_mutex.unlock();
    return mimeTypes;
}

void QMimeDatabase::syncUserModifiedMimeTypes()
{
    m_d->m_mutex.lock();
    m_d->syncUserModifiedMimeTypes();
    m_d->m_mutex.unlock();
}

void QMimeDatabase::clearUserModifiedMimeTypes()
{
    m_d->m_mutex.lock();
    m_d->clearUserModifiedMimeTypes();
    m_d->m_mutex.unlock();
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
    m_d->m_mutex.lock();
    const bool rc = m_d->setPreferredSuffix(typeOrAlias, suffix);
    m_d->m_mutex.unlock();
    return rc;
}

QList<MimeGlobPattern> QMimeDatabase::toGlobPatterns(const QStringList &patterns, int weight)
{
    return QMimeDatabasePrivate::toGlobPatterns(patterns, weight);
}

QStringList QMimeDatabase::fromGlobPatterns(const QList<MimeGlobPattern> &globPatterns)
{
    return QMimeDatabasePrivate::fromGlobPatterns(globPatterns);
}

QDebug operator<<(QDebug d, const QMimeDatabase &mt)
{
    QString s;
    {
        QTextStream str(&s);
        mt.m_d->debug(str);
    }
    d << s;
    return d;
}

QT_END_NAMESPACE
