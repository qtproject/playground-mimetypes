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

#include "qmimeglobpattern_p.h"

#include <QtCore/QLocale>

#include "magicmatcher_p.h"

QT_BEGIN_NAMESPACE

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

QMimeTypeData::QMimeTypeData(const QMimeType &other)
    : suffixPattern(other.d->suffixPattern)
    , name(other.d->name)
    , comment(other.d->comment)
    , localeComments(other.d->localeComments)
    , aliases(other.d->aliases)
    , genericIconName(other.d->genericIconName)
    , globPatterns(other.d->globPatterns)
    , subClassOf(other.d->subClassOf)
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
    aliases.clear();
    globPatterns.clear();
    subClassOf.clear();
    preferredSuffix.clear();
    suffixes.clear();
    magicMatchers.clear();
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
}

#if 0
unsigned QMimeTypeData::matchesFileBySuffix(const QString &fileName) const
{
    foreach (const QString &pattern, globPatterns) {
        QMimeGlobPattern glob(pattern, name);
        if (glob.matchFileName(fileName))
            return glob.weight();
    }

    return 0;
}
#endif

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
    return d == other.d;
}

void QMimeType::clear()
{
    d->clear();
}

bool QMimeType::isValid() const
{
    return !d->name.isEmpty();
}

const QString &QMimeType::name() const
{
    return d->name;
}

const QString &QMimeType::comment() const
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

const QStringList &QMimeType::aliases() const
{
    return d->aliases;
}

const QString &QMimeType::genericIconName() const
{
    return d->genericIconName;
}

const QStringList &QMimeType::globPatterns() const
{
    return d->globPatterns;
}

const QStringList &QMimeType::subClassOf() const
{
    return d->subClassOf;
}

/*!
    Returns the known suffixes for the MIME type.

    Extension over standard MIME data
*/
const QStringList &QMimeType::suffixes() const
{
    return d->suffixes;
}

/*!
    Returns the preferred suffix for the MIME type.

    Extension over standard MIME data
*/
const QString &QMimeType::preferredSuffix() const
{
    return d->preferredSuffix;
}

#if 0   // Seems unused
/*!
    Checks for \a name or one of the aliases.
*/
bool QMimeType::matchesName(const QString &name) const
{
    return d->name == name || d->aliases.contains(name) /* TODO: BROKEN! MUST COMPARE WITH d->name */;
}
#endif

unsigned QMimeType::matchesData(const QByteArray &data) const
{
    return d->matchesData(data);
}

#if 0
/*!
    Checks the glob pattern weights and magic priorities so the highest
    value is returned. A 0 (zero) indicates no match.
*/
unsigned QMimeType::matchesFile(QIODevice *device, const QString &fileName) const
{
    FileMatchContext context(device, fileName);
    const unsigned suffixPriority = d->matchesFileBySuffix(context.fileName());
    if (suffixPriority >= QMimeGlobPattern::MaxWeight)
        return QMimeGlobPattern::MaxWeight;
    return qMax(suffixPriority, d->matchesData(context.data()));
}

/*!
    Performs search by glob patterns.
*/
unsigned QMimeType::matchesFileBySuffix(const QString &fileName) const
{
    return d->matchesFileBySuffix(fileName);
}
#endif

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


QT_END_NAMESPACE
