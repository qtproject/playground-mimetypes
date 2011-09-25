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

#include "magicmatcher_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMimeGlobPattern
    \brief Glob pattern for file names for MIME type matching.

    \sa QMimeType, QMimeDatabase, QMimeMagicRuleMatcher, QMimeMagicRule
    \sa BinaryMatcher, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
*/

bool QMimeGlobPattern::matchFileName(const QString& _filename) const
{
    // "Applications MUST match globs case-insensitively, except when the case-sensitive
    // attribute is set to true."
    // QMimeGlobPattern takes care of putting case-insensitive patterns in lowercase.
    const QString filename = m_caseSensitivity == Qt::CaseInsensitive ? _filename.toLower() : _filename;

    const int pattern_len = m_pattern.length();
    if (!pattern_len)
        return false;
    const int len = filename.length();

    const int starCount = m_pattern.count(QLatin1Char('*'));

    // Patterns like "*~", "*.extension"
    if (m_pattern[0] == QLatin1Char('*') && m_pattern.indexOf(QLatin1Char('[')) == -1 && starCount == 1)
    {
        if ( len + 1 < pattern_len ) return false;

        const QChar *c1 = m_pattern.unicode() + pattern_len - 1;
        const QChar *c2 = filename.unicode() + len - 1;
        int cnt = 1;
        while (cnt < pattern_len && *c1-- == *c2--)
            ++cnt;
        return cnt == pattern_len;
    }

    // Patterns like "README*" (well this is currently the only one like that...)
    if (starCount == 1 && m_pattern.at(pattern_len - 1) == QLatin1Char('*')) {
        if ( len + 1 < pattern_len ) return false;
        if (m_pattern.at(0) == QLatin1Char('*'))
            return filename.indexOf(m_pattern.mid(1, pattern_len - 2)) != -1;

        const QChar *c1 = m_pattern.unicode();
        const QChar *c2 = filename.unicode();
        int cnt = 1;
        while (cnt < pattern_len && *c1++ == *c2++)
           ++cnt;
        return cnt == pattern_len;
    }

    // Names without any wildcards like "README"
    if (m_pattern.indexOf(QLatin1Char('[')) == -1 && starCount == 0 && m_pattern.indexOf(QLatin1Char('?')))
        return (m_pattern == filename);

    // Other (quite rare) patterns, like "*.anim[1-9j]": use slow but correct method
    const QRegExp rx(m_pattern, Qt::CaseSensitive, QRegExp::WildcardUnix);
    return rx.exactMatch(filename);
}

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
unsigned QMimeTypeData::matchesFileBySuffix(const QString &name) const
{
    foreach (const QMimeGlobPattern &glob, globPatterns) {
        if (glob.matchFileName(name))
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

QStringList QMimeType::globPatterns() const
{
    return d->globPatterns;
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
    return d->type == type || d->aliases.contains(type) /* TODO: BROKEN! MUST COMPARE WITH d->type */;
}

unsigned QMimeType::matchesData(const QByteArray &data) const
{
    return d->matchesData(data);
}

#if 0
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

static bool isFastPattern(const QString& pattern)
{
   // starts with "*.", has no other '*' and no other '.'
   return pattern.lastIndexOf(QLatin1Char('*')) == 0
      && pattern.lastIndexOf(QLatin1Char('.')) == 1
      // and contains no other special character
      && !pattern.contains(QLatin1Char('?'))
      && !pattern.contains(QLatin1Char('['))
      ;
}

void QMimeAllGlobPatterns::addGlob(const QMimeGlobPattern& glob)
{
    const QString &pattern = glob.pattern();
    Q_ASSERT(!pattern.isEmpty());

    //kDebug() << "pattern" << pattern << "glob.weight=" << glob.weight() << "isFast=" << isFastPattern(pattern) << glob.flags;

    // Store each patterns into either m_fastPatternDict (*.txt, *.html etc. with default weight 50)
    // or for the rest, like core.*, *.tar.bz2, *~, into highWeightPatternOffset (>50)
    // or lowWeightPatternOffset (<=50)

    if (glob.weight() == 50 && isFastPattern(pattern) && !glob.isCaseSensitive()) {
        // The bulk of the patterns is *.foo with weight 50 --> those go into the fast patterns hash.
        const QString extension = pattern.mid(2).toLower();
        QStringList& patterns = m_fastPatterns[extension]; // find or create
        // This would just slow things down: if (!patterns.contains(glob.mimeType()))
        patterns.append(glob.mimeType());
    } else {
        if (glob.weight() > 50) {
            // This would just slow things down: if (!m_highWeightGlobs.hasPattern(glob.mimeType(), glob.pattern()))
            m_highWeightGlobs.append(glob);
        } else {
            //This would just slow things down: if (!m_lowWeightGlobs.hasPattern(glob.mimeType(), glob.pattern()))
            m_lowWeightGlobs.append(glob);
        }
    }
}

void QMimeAllGlobPatterns::removeMime(const QString& mime)
{
    QMutableHashIterator<QString, QStringList> it(m_fastPatterns);
    while (it.hasNext()) {
        it.next().value().removeAll(mime);
    }
    m_highWeightGlobs.removeMime(mime);
    m_lowWeightGlobs.removeMime(mime);
}

QT_END_NAMESPACE
