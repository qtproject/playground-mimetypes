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

#ifndef QMIMETYPE_P_H
#define QMIMETYPE_P_H

#include "qmimetype.h"

#include <QtCore/QHash>

QT_BEGIN_NAMESPACE

class QTextStream;

class QMimeMagicRuleMatcher;

class QMimeGlobPattern
{
public:
    static const unsigned MaxWeight = 100;
    static const unsigned DefaultWeight = 50;
    static const unsigned MinWeight = 1;

    explicit QMimeGlobPattern(const QString &pattern, const QString &mimeType, unsigned weight = DefaultWeight, Qt::CaseSensitivity s = Qt::CaseInsensitive) :
        m_pattern(pattern), m_mimeType(mimeType), m_weight(weight), m_caseSensitivity(s) {
        if (s == Qt::CaseInsensitive) {
            m_pattern = m_pattern.toLower();
        }
    }
    ~QMimeGlobPattern() {}

    bool matchFileName(const QString& filename) const;

    inline const QString& pattern() const
    { return m_pattern; }
    inline unsigned weight() const
    { return m_weight; }
    inline const QString& mimeType() const
    { return m_mimeType; }
    inline bool isCaseSensitive() const
    { return m_caseSensitivity == Qt::CaseSensitive; }

private:
    QString m_pattern;
    QString m_mimeType;
    int m_weight;
    Qt::CaseSensitivity m_caseSensitivity;
};

class QMimeGlobPatternList : public QList<QMimeGlobPattern>
{
public:
    bool hasPattern(const QString& mime, const QString& pattern) const {
        const_iterator it = begin();
        const const_iterator myend = end();
        for (; it != myend; ++it)
            if ((*it).pattern() == pattern && (*it).mimeType() == mime)
                return true;
        return false;
    }
    // "noglobs" is very rare occurrence, so it's ok if it's slow
    void removeMime(const QString& mime) {
        QMutableListIterator<QMimeGlobPattern> it(*this);
        while (it.hasNext()) {
            if (it.next().mimeType() == mime)
                it.remove();
        }
    }
};

/**
 * Result of the globs parsing, as data structures ready for efficient mimetype matching.
 * This contains:
 * 1) a map of fast regular patterns (e.g. *.txt is stored as "txt" in a qhash's key)
 * 2) a linear list of high-weight globs
 * 3) a linear list of low-weight globs
 */
class QMimeAllGlobPatterns
{
public:
    typedef QHash<QString, QStringList> PatternsMap; // mimetype -> patterns

    void addGlob(const QMimeGlobPattern& glob);
    void removeMime(const QString& mime);

    PatternsMap m_fastPatterns; // example: "doc" -> "application/msword", "text/plain"
    QMimeGlobPatternList m_highWeightGlobs;
    QMimeGlobPatternList m_lowWeightGlobs; // <= 50, including the non-fast 50 patterns
};

class QMimeTypeData : public QSharedData
{
public:
    typedef QHash<QString, QString> LocaleHash;

    QMimeTypeData();

    void clear();
    void addGlobPattern(const QString &pattern);

    unsigned matchesFileBySuffix(const QString &name) const;
    unsigned matchesData(const QByteArray &data) const;

    const QRegExp suffixPattern;

    QString type;
    QString comment;
    LocaleHash localeComments;
    QStringList aliases;
    QString genericIconName;
    QStringList globPatterns;
    QStringList subClassOf;
    QString preferredSuffix;
    QStringList suffixes;
    QList<QMimeMagicRuleMatcher> magicMatchers;
};

QT_END_NAMESPACE

#endif // QMIMETYPE_P_H
