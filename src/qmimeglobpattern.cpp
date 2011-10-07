#include "qmimeglobpattern_p.h"
#include <QRegExp>
#include <QStringList>
#include <QDebug>

/*!
    \class QMimeGlobPattern
    \brief Glob pattern for file names for MIME type matching.

    \sa QMimeType, QMimeDatabase, QMimeMagicRuleMatcher, QMimeMagicRule
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

void QMimeGlobPatternList::match(QStringList &matchingMimeTypes,
                                 const QString &fileName,
                                 QString *foundExt) const
{
    int matchingPatternLength = 0;
    qint32 lastMatchedWeight = 0;
    if (!matchingMimeTypes.isEmpty()) {
        // We found matches in the fast pattern dict already:
        matchingPatternLength = foundExt->length() + 2; // *.foo -> length=5
        lastMatchedWeight = 50;
    }

    QMimeGlobPatternList::const_iterator it = this->constBegin();
    const QMimeGlobPatternList::const_iterator end = this->constEnd();
    for ( ; it != end; ++it ) {
        const QMimeGlobPattern &glob = *it;
        if (glob.matchFileName(fileName)) {
            const int weight = glob.weight();
            const QString pattern = glob.pattern();
            // Is this a lower-weight pattern than the last match? Stop here then.
            if (weight < lastMatchedWeight)
                break;
            if (lastMatchedWeight > 0 && weight > lastMatchedWeight) // can't happen
                qWarning() << "Assumption failed; globs2 weights not sorted correctly"
                           << weight << ">" << lastMatchedWeight;
            // Is this a shorter or a longer match than an existing one, or same length?
            if (pattern.length() < matchingPatternLength) {
                continue; // too short, ignore
            } else if (pattern.length() > matchingPatternLength) {
                // longer: clear any previous match (like *.bz2, when pattern is *.tar.bz2)
                matchingMimeTypes.clear();
                // remember the new "longer" length
                matchingPatternLength = pattern.length();
            }
            matchingMimeTypes.push_back(glob.mimeType());
            if (pattern.startsWith(QLatin1String("*.")))
                *foundExt = pattern.mid(2);
        }
    }
}

QStringList QMimeAllGlobPatterns::matchingGlobs(const QString &fileName, QString *foundExt) const
{
    // First try the high weight matches (>50), if any.
    QStringList matchingMimeTypes;
    m_highWeightGlobs.match(matchingMimeTypes, fileName, foundExt);
    if (matchingMimeTypes.isEmpty()) {

        // Now use the "fast patterns" dict, for simple *.foo patterns with weight 50
        // (which is most of them, so this optimization is definitely worth it)
        const int lastDot = fileName.lastIndexOf(QLatin1Char('.'));
        if (lastDot != -1) { // if no '.', skip the extension lookup
            const int ext_len = fileName.length() - lastDot - 1;
            const QString simpleExtension = fileName.right( ext_len ).toLower();
            // (toLower because fast patterns are always case-insensitive and saved as lowercase)

            matchingMimeTypes = m_fastPatterns.value(simpleExtension);
            if (!matchingMimeTypes.isEmpty()) {
                *foundExt = simpleExtension;
                // Can't return yet; *.tar.bz2 has to win over *.bz2, so we need the low-weight mimetypes anyway,
                // at least those with weight 50.
            }
        }

        // Finally, try the low weight matches (<=50)
        m_lowWeightGlobs.match(matchingMimeTypes, fileName, foundExt);
    }
    return matchingMimeTypes;
}
