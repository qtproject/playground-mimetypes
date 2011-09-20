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
#include "qmimedatabase_p.h"
#include "magicmatcher_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMimeGlobPattern
    \brief Glob pattern for file names for MIME type matching.

    \sa QMimeType, QMimeDatabase, QMimeMagicRuleMatcher, QMimeMagicRule
    \sa BinaryMatcher, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
*/

/*!
    \var QMimeTypeData::suffixPattern
    \brief Regular expression to match a suffix glob pattern: "*.ext" (and not sth like "Makefile" or "*.log[1-9]"
*/

QMimeTypeData::QMimeTypeData()
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
        suffixes.append(suffix);
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
    foreach (const QMimeGlobPattern &glob, globPatterns) {
        if (glob.regExp().exactMatch(name))
            return glob.weight();
    }

    return 0;
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

unsigned QMimeTypeData::matchesData(const QByteArray &data) const
{
    unsigned priority = 0;
    if (!data.isEmpty()) {
        // TODO: discuss - this code is slow :(
        // Hack for text/plain and application/octet-stream
        if (magicMatchers.isEmpty()) {
            if (type == QLatin1String("text/plain") && isTextFile(data))
                priority = 2;
            else if (type == QLatin1String("application/octet-stream"))
                priority = 1;
        } else {
            foreach (const QMimeMagicRuleMatcher &matcher, magicMatchers) {
                if (matcher.priority() > priority && matcher.matches(data))
                    priority = matcher.priority();
            }
        }
    }
    return priority;
}

/*!
    \class QMimeType

    \brief MIME type data used in Qt Creator.

    Contains most information from standard MIME type XML database files.

    In addition, the class provides a list of suffixes and a concept of the
    'preferred suffix' (derived from glob patterns). This is used for example
    to be able to configure the suffix used for C++-files in Qt Creator.

    MIME XML looks like:
    \code
    <?xml version="1.0" encoding="UTF-8"?>
    <mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
    <!-- MIME types must match the desktop file associations -->
      <mime-type type="application/vnd.nokia.qt.qmakeprofile">
        <comment xml:lang="en">Qt qmake Profile</comment>
        <glob pattern="*.pro" weight="50"/>
      </mime-type>
    </mime-info>
    \endcode

    \sa QMimeDatabase, QMimeMagicRuleMatcher, QMimeMagicRule, QMimeGlobPattern
*/

QMimeType::QMimeType() :
    d(new QMimeTypeData)
{
}

QMimeType::QMimeType(const QMimeType &other) :
    d(other.d)
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
    if (d != other.d)
        d = other.d;
    return *this;
}

bool QMimeType::operator==(const QMimeType &other) const
{
    return d == other.d;
}

void QMimeType::clear()
{
    d->clear();
}

bool QMimeType::isValid() const
{
    return !d->type.isEmpty();
}

QString QMimeType::type() const
{
    return d->type;
}

QString QMimeType::comment() const
{
    return d->comment;
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

/*!
    \param localeArg en, de...
*/
QString QMimeType::localeComment(const QString &localeArg) const
{
    const QString locale = localeArg.isEmpty() ? systemLanguage() : localeArg;
    return d->localeComments.value(locale, d->comment);
}

QStringList QMimeType::aliases() const
{
    return d->aliases;
}

QString QMimeType::genericIconName() const
{
    return d->genericIconName;
}

QList<QMimeGlobPattern> QMimeType::weightedGlobPatterns() const
{
    return d->globPatterns;
}

QStringList QMimeType::globPatterns() const
{
    return QMimeDatabasePrivate::fromGlobPatterns(d->globPatterns);
}

QList<QMimeMagicRuleMatcher> QMimeType::magicMatchers() const
{
    return d->magicMatchers;
}

QStringList QMimeType::subClassOf() const
{
    return d->subClassOf;
}

/*!
    Returns the known suffixes for the MIME type.

    Extension over standard MIME data
*/
QStringList QMimeType::suffixes() const
{
    return d->suffixes;
}

/*!
    Returns the preferred suffix for the MIME type.

    Extension over standard MIME data
*/
QString QMimeType::preferredSuffix() const
{
    return d->preferredSuffix;
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

/*!
    Performs search by glob patterns.
*/
unsigned QMimeType::matchesName(const QString &name) const
{
    return d->matchesFileBySuffix(name);
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
    \class QMutableMimeType

    \brief Class is used to create user MIME types.
*/
QMutableMimeType::QMutableMimeType() :
    QMimeType()
{
}

QMutableMimeType::QMutableMimeType(const QString &type) :
    QMimeType()
{
    setType(type);
}

QMutableMimeType::QMutableMimeType(const QMimeType &other) :
    QMimeType(other)
{
    d.detach();
}

QMutableMimeType::QMutableMimeType(const QMutableMimeType &other) :
    QMimeType(other)
{
}

QMutableMimeType::~QMutableMimeType()
{
}

void QMutableMimeType::setType(const QString &type)
{
    d->type = type;
}

void QMutableMimeType::setComment(const QString &comment)
{
    d->comment = comment;
}

void QMutableMimeType::setLocaleComment(const QString &locale, const QString &comment)
{
    if (locale.isEmpty())
        return;

     d->localeComments[locale] = comment;
}

void QMutableMimeType::setAliases(const QStringList &aliases)
{
     d->aliases = aliases;
}

void QMutableMimeType::setGenericIconName(const QString &genericIconName)
{
    d->genericIconName = genericIconName;
}

void QMutableMimeType::setWeightedGlobPatterns(const QList<QMimeGlobPattern> &globPatterns)
{
    d->globPatterns = globPatterns;

    QString oldPrefferedSuffix = d->preferredSuffix;
    d->suffixes.clear();
    d->preferredSuffix.clear();
    d->assignSuffixes(QMimeDatabasePrivate::fromGlobPatterns(globPatterns));
    if (d->preferredSuffix != oldPrefferedSuffix && d->suffixes.contains(oldPrefferedSuffix))
        d->preferredSuffix = oldPrefferedSuffix;
}

void QMutableMimeType::addMagicMatcher(const QMimeMagicRuleMatcher &matcher)
{
    d->magicMatchers.append(matcher);
}

void QMutableMimeType::setMagicMatchers(const QList<QMimeMagicRuleMatcher> &matchers)
{
    d->magicMatchers = matchers;
}

void QMutableMimeType::setSubClassOf(const QStringList &subClassOf)
{
    d->subClassOf = subClassOf;
}

bool QMutableMimeType::setPreferredSuffix(const QString &preferredSuffix)
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

QT_END_NAMESPACE
