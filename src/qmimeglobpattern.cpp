#include "qmimeglobpattern_p.h"
#include <QRegExp>
#include <QStringList>

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
