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

#include "qmimetype.h"
#include "qmimetype_p.h"

#include <QtCore/QLocale>

#include "qmimedatabase.h"
#include "magicmatcher_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMimeGlobPattern
    \brief Glob pattern for file names for mime type matching.

    \sa QMimeType, QMimeDatabase, MagicRuleMatcher, QMimeMagicRule
    \sa BinaryMatcher, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
*/

QMimeGlobPattern::QMimeGlobPattern(const QRegExp &regExp, unsigned weight) :
    m_regExp(regExp), m_weight(weight)
{
}

QMimeGlobPattern::~QMimeGlobPattern()
{
}


QMimeTypeData::QMimeTypeData()
    // RE to match a suffix glob pattern: "*.ext" (and not sth like "Makefile" or
    // "*.log[1-9]"
    : suffixPattern(QLatin1String("^\\*\\.[\\w+]+$"))
{
    if (!suffixPattern.isValid())
        qWarning("MimeTypeData(): invalid suffixPattern");
}

void QMimeTypeData::clear()
{
    type.clear();
    comment.clear();
    aliases.clear();
    globPatterns.clear();
    subClassOf.clear();
    preferredSuffix.clear();
    suffixes.clear();
    magicMatchers.clear();
}

void QMimeTypeData::assignSuffix(const QString &pattern)
{
    if (suffixPattern.exactMatch(pattern)) {
        const QString suffix = pattern.right(pattern.size() - 2);
        suffixes.push_back(suffix);
        if (preferredSuffix.isEmpty())
            preferredSuffix = suffix;
    }
}

void QMimeTypeData::assignSuffixes(const QStringList &patterns)
{
    foreach (const QString &pattern, patterns)
        assignSuffix(pattern);
}

unsigned QMimeTypeData::matchesFileBySuffix(const QString &name) const
{
    // check globs
    foreach (const QMimeGlobPattern &glob, globPatterns) {
        if (glob.regExp().exactMatch(name))
            return glob.weight();
    }

    return 0;
}

unsigned QMimeTypeData::matchesData(const QByteArray &data) const
{
    unsigned priority = 0;
    if (!data.isEmpty()) {
        foreach (const QMimeMagicRuleMatcher &matcher, magicMatchers) {
            if (matcher.priority() > priority && matcher.matches(data))
                priority = matcher.priority();
        }
    }
    return priority;
}


/*!
    \class MimeType

    \brief Mime type data used in Qt Creator.

    Contains most information from standard mime type XML database files.

    In addition, the class provides a list of suffixes and a concept of the
    'preferred suffix' (derived from glob patterns). This is used for example
    to be able to configure the suffix used for C++-files in Qt Creator.

    Mime XML looks like:
    \code
    <?xml version="1.0" encoding="UTF-8"?>
    <mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
    <!-- Mime types must match the desktop file associations -->
      <mime-type type="application/vnd.nokia.qt.qmakeprofile">
        <comment xml:lang="en">Qt qmake Profile</comment>
        <glob pattern="*.pro" weight="50"/>
      </mime-type>
    </mime-info>
    \endcode

    \sa QMimeDatabase, MagicRuleMatcher, QMimeMagicRule, QMimeGlobPattern
    \sa BinaryMatcher, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
*/

QMimeType::QMimeType()
    : d(new QMimeTypeData)
{
}

QMimeType::QMimeType(const QString &type)
    : d(new QMimeTypeData)
{
    d->type = type;
}

QMimeType::QMimeType(const QString &type,
                     const QList<QMimeMagicRuleMatcher> &matchers,
                     const QList<QMimeGlobPattern> &globPatterns,
                     const QStringList &subClassOf)
    : d(new QMimeTypeData)
{
    d->type = type;
    d->magicMatchers = matchers;
    if (!globPatterns.isEmpty())
        setGlobPatterns(globPatterns);
    d->subClassOf = subClassOf;
}

QMimeType::QMimeType(const QMimeType &other)
    : d(other.d)
{
}

QMimeType::QMimeType(const QMimeTypeData &dd)
    : d(new QMimeTypeData(dd))
{
}

QMimeType::~QMimeType()
{
}

QMimeType &QMimeType::operator=(const QMimeType &other)
{
    if (this != &other)
        d = other.d;
    return *this;
}

void QMimeType::clear()
{
    d->clear();
}

bool QMimeType::isValid() const
{
    return !d->type.isEmpty();
}

bool QMimeType::isTopLevel() const
{
    return d->subClassOf.empty();
}

QString QMimeType::type() const
{
    return d->type;
}

void QMimeType::setType(const QString &type)
{
    d->type = type;
}

QString QMimeType::comment() const
{
    return d->comment;
}

void QMimeType::setComment(const QString &comment)
{
    d->comment = comment;
}

// Return "en", "de", etc. derived from "en_US", de_DE".
static inline QString systemLanguage()
{
    QString name = QLocale::system().name();
    const int underScorePos = name.indexOf(QLatin1Char('_'));
    if (underScorePos != -1)
        name.truncate(underScorePos);
    return name;
}

QString QMimeType::localeComment(const QString &localeArg) const
{
    const QString locale = localeArg.isEmpty() ? systemLanguage() : localeArg;
    return d->localeComments.value(locale, d->comment);
}

void QMimeType::setLocaleComment(const QString &locale, const QString &comment)
{
    if (locale.isEmpty())
        return;

     d->localeComments[locale] = comment;
}

QStringList QMimeType::aliases() const
{
    return d->aliases;
}

void QMimeType::setAliases(const QStringList &aliases)
{
     d->aliases = aliases;
}

QString QMimeType::genericIconName() const
{
    return d->genericIconName;
}

void QMimeType::setGenericIconName(const QString &genericIconName)
{
    d->genericIconName = genericIconName;
}

QList<QMimeGlobPattern> QMimeType::globPatterns() const
{
    return d->globPatterns;
}

void QMimeType::setGlobPatterns(const QList<QMimeGlobPattern> &globPatterns)
{
    d->globPatterns = globPatterns;

    QString oldPrefferedSuffix = d->preferredSuffix;
    d->suffixes.clear();
    d->preferredSuffix.clear();
    d->assignSuffixes(QMimeDatabase::fromGlobPatterns(globPatterns));
    if (d->preferredSuffix != oldPrefferedSuffix && d->suffixes.contains(oldPrefferedSuffix))
        d->preferredSuffix = oldPrefferedSuffix;
}

QStringList QMimeType::subClassOf() const
{
    return d->subClassOf;
}

void QMimeType::setSubClassOf(const QStringList &subClassOf)
{
    d->subClassOf = subClassOf;
}

QString QMimeType::preferredSuffix() const
{
    return d->preferredSuffix;
}

bool QMimeType::setPreferredSuffix(const QString &preferredSuffix)
{
    if (!d->suffixes.contains(preferredSuffix)) {
        qWarning("%s: Attempt to set preferred suffix to '%s', which is not in the list of suffixes: %s.",
                 d->type.toLocal8Bit().constData(),
                 preferredSuffix.toLocal8Bit().constData(),
                 d->suffixes.join(QLatin1String(", ")).toLocal8Bit().constData());
        return false;
    }
    d->preferredSuffix = preferredSuffix;
    return true;
}

/*!
    Returns a filter string usable for a file dialog.
*/
QString QMimeType::filterString() const
{
    QString filter;

    if (!d->globPatterns.empty()) { // !Binary files
        // ### todo: Use localeComment() once creator is shipped with translations
        filter += d->comment + QLatin1String(" (");
        for (int i = 0; i < d->globPatterns.size(); ++i) {
            if (i != 0)
                filter += QLatin1Char(' ');
            filter += d->globPatterns.at(i).regExp().pattern();
        }
        filter +=  QLatin1Char(')');
    }

    return filter;
}

/*!
    Checks for \a type or one of the aliases.
*/
bool QMimeType::matchesType(const QString &type) const
{
    return d->type == type || d->aliases.contains(type);
}

unsigned QMimeType::matchesData(const QByteArray &data) const
{
    return d->matchesData(data);
}

/*!
    Checks the glob pattern weights and magic priorities so the highest
    value is returned. A 0 (zero) indicates no match.
*/
unsigned QMimeType::matchesFile(const QFileInfo &file) const
{
    FileMatchContext context(file);
    const unsigned suffixPriority = d->matchesFileBySuffix(context.fileName());
    if (suffixPriority >= QMimeGlobPattern::MaxWeight)
        return QMimeGlobPattern::MaxWeight;
    return qMax(suffixPriority, d->matchesData(context.data()));
}

QStringList QMimeType::suffixes() const
{
    return d->suffixes;
}

void QMimeType::addMagicMatcher(const QMimeMagicRuleMatcher &matcher)
{
    d->magicMatchers.push_back(matcher);
}

QList<QMimeMagicRuleMatcher> QMimeType::magicMatchers() const
{
    return d->magicMatchers;
}

void QMimeType::setMagicMatchers(const QList<QMimeMagicRuleMatcher> &matchers)
{
    d->magicMatchers = matchers;
}

QT_END_NAMESPACE
