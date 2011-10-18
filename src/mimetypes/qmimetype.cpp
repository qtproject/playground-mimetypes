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
#include "qmimedatabase_p.h"
#include "qmimeprovider_p.h"

#include "qmimeglobpattern_p.h"

//#include <QtCore/QDebug>
#include <QtCore/QLocale>

#include "magicmatcher_p.h"

QT_BEGIN_NAMESPACE

/*!
    \var QMimeTypeData::suffixPattern
    \brief Regular expression to match a suffix glob pattern: "*.ext" or "*.ext1.ext2" (and not sth like "Makefile" or "*.log[1-9]"
*/

QMimeTypeData::QMimeTypeData()
    : suffixPattern(QLatin1String("^\\*[\\.\\w+]+$"))
{
    if (!suffixPattern.isValid())
        qWarning("MimeTypeData(): invalid suffixPattern");
}

QMimeTypeData::QMimeTypeData(const QMimeType &other)
    : suffixPattern(other.d->suffixPattern)
    , name(other.d->name)
    , comment(other.d->comment)
    , localeComments(other.d->localeComments)
    , aliases(other.d->aliases)
    , genericIconName(other.d->genericIconName)
    , iconName(other.d->iconName)
    , globPatterns(other.d->globPatterns)
    , preferredSuffix(other.d->preferredSuffix)
    , suffixes(other.d->suffixes)
    , magicMatchers(other.d->magicMatchers)
{
    if (!suffixPattern.isValid())
        qWarning("MimeTypeData(): invalid suffixPattern");
}

void QMimeTypeData::clear()
{
    name.clear();
    comment.clear();
    localeComments.clear();
    aliases.clear();
    genericIconName.clear();
    iconName.clear();
    globPatterns.clear();
    preferredSuffix.clear();
    suffixes.clear();
    magicMatchers.clear();
}

bool QMimeTypeData::operator==(const QMimeTypeData &other) const
{
    return name == other.name &&
           comment == other.comment &&
           localeComments == other.localeComments &&
           aliases == other.aliases &&
           genericIconName == other.genericIconName &&
           iconName == other.iconName &&
           globPatterns == other.globPatterns &&
           preferredSuffix == other.preferredSuffix &&
           suffixes == other.suffixes &&
           magicMatchers == other.magicMatchers;
}

void QMimeTypeData::addGlobPattern(const QString &pattern)
{
    globPatterns.append(pattern);
    if (suffixPattern.exactMatch(pattern)) {
        const QString suffix = pattern.right(pattern.size() - 2);
        suffixes.append(suffix);
        if (preferredSuffix.isEmpty())
            preferredSuffix = suffix;
    }
    else {
        //qDebug() << Q_FUNC_INFO << "Skipping suffix" << pattern;
    }
}

static inline bool isTextFile(const QByteArray &data)
{
    // UTF16 byte order marks
    static const char bigEndianBOM[] = "\xFE\xFF";
    static const char littleEndianBOM[] = "\xFF\xFE";
    if (data.startsWith(bigEndianBOM) || data.startsWith(littleEndianBOM))
        return true;

    // Check the first 32 bytes (see shared-mime spec)
    const char *p = data.constData();
    const char *e = p + data.size();
    for ( ; p < e; ++p) {
        if ((unsigned char)(*p) < 32 && *p != 9 && *p !=10 && *p != 13)
            return false;
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
            if (name == QLatin1String("text/plain") && isTextFile(data))
                priority = 2;
            else if (name == QLatin1String("application/octet-stream"))
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
    return d == other.d ||
           *d == *other.d;
}

bool QMimeType::isValid() const
{
    return !d->name.isEmpty();
}

QString QMimeType::name() const
{
    return d->name;
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

QString QMimeType::iconName() const
{
    if (d->iconName.isEmpty()) {
        // Make default icon name from the mimetype name
        d->iconName = name();
        const int slashindex = d->iconName.indexOf(QLatin1Char('/'));
        if (slashindex != -1)
            d->iconName[slashindex] = QLatin1Char('-');
    }
    return d->iconName;
}

QStringList QMimeType::globPatterns() const
{
    return d->globPatterns;
}

QStringList QMimeType::parentMimeTypes() const
{
    return QMimeDatabasePrivate::instance()->provider()->parents(d->name);
}

static void collectParentMimeTypes(const QString& mime, QStringList& allParents)
{
    QStringList parents = QMimeDatabasePrivate::instance()->provider()->parents(mime);
    foreach(const QString& parent, parents) {
        // I would use QSet, but since order matters I better not
        if (!allParents.contains(parent))
            allParents.append(parent);
    }
    // We want a breadth-first search, so that the least-specific parent (octet-stream) is last
    // This means iterating twice, unfortunately.
    foreach(const QString& parent, parents) {
        collectParentMimeTypes(parent, allParents);
    }
}

QStringList QMimeType::allParentMimeTypes() const
{
    QStringList allParents;
    const QString canonical = d->name; // TODO QMimeDatabasePrivate::instance()->resolveAlias(d->name);
    if (!canonical.isEmpty())
        allParents.append(canonical);
    collectParentMimeTypes(d->name, allParents);

    return allParents;
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
    Returns a filter string usable for a file dialog.
*/
QString QMimeType::filterString() const
{
    QString filter;

    if (!d->globPatterns.empty()) { // !Binary files
        filter += localeComment() + QLatin1String(" (");
        for (int i = 0; i < d->globPatterns.size(); ++i) {
            if (i != 0)
                filter += QLatin1Char(' ');
            filter += d->globPatterns.at(i);
        }
        filter +=  QLatin1Char(')');
    }

    return filter;
}

/*!
    Returns true if this mimetype is \name, or inherits \name,
    or is an alias for \name.
 */
bool QMimeType::inherits(const QString &mimeTypeName) const
{
    if (d->name == mimeTypeName)
        return true;
    return QMimeDatabasePrivate::instance()->inherits(d->name, mimeTypeName);
}


QT_END_NAMESPACE
