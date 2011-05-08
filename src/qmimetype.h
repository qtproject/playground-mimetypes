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

#ifndef QMIMETYPE_H
#define QMIMETYPE_H

#include "qmimetype_global.h"

#include "magicmatcher.h"

#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QFileInfo;

class QMIMETYPE_EXPORT MimeGlobPattern
{
public:
    static const unsigned MaxWeight = 100;
    static const unsigned MinWeight = 1;

    explicit MimeGlobPattern(const QRegExp &regExp, unsigned weight = MaxWeight);
    ~MimeGlobPattern();

    const QRegExp &regExp() const;
    unsigned weight() const;

private:
    QRegExp m_regExp;
    int m_weight;
};

class QMimeTypeData;
class QMIMETYPE_EXPORT QMimeType
{
public:
    typedef IMagicMatcher::IMagicMatcherList IMagicMatcherList;
    typedef IMagicMatcher::IMagicMatcherSharedPointer IMagicMatcherSharedPointer;

    QMimeType();
    QMimeType(const QMimeType&);
    QMimeType &operator=(const QMimeType&);
    ~QMimeType();

    void clear();
    bool isNull() const;
    operator bool() const;

    bool isTopLevel() const;

    QString type() const;
    void setType(const QString &type);

    QStringList aliases() const;
    void setAliases(const QStringList &);

    QString comment() const;
    void setComment(const QString &comment);

    QString localeComment(const QString &locale = QString() /* en, de...*/) const;
    void setLocaleComment(const QString &locale, const QString &comment);

    QList<MimeGlobPattern> globPatterns() const;
    void setGlobPatterns(const QList<MimeGlobPattern> &);

    QStringList subClassesOf() const;
    void setSubClassesOf(const QStringList &);

    // Extension over standard mime data
    QStringList suffixes() const;
    QString preferredSuffix() const;
    bool setPreferredSuffix(const QString&);

    // Check for type or one of the aliases
    bool matchesType(const QString &type) const;

    // Check glob patterns weights and magic priorities so the highest
    // value is returned. A 0 (zero) indicates no match.
    unsigned matchesFile(const QFileInfo &file) const;

    // Return a filter string usable for a file dialog
    QString filterString() const;

    void addMagicMatcher(const IMagicMatcherSharedPointer &matcher);

    const IMagicMatcherList &magicMatchers() const;
    void setMagicMatchers(const IMagicMatcherList &matchers);

    // Convenience for rule-base matchers.
    IMagicMatcherList magicRuleMatchers() const;
    void setMagicRuleMatchers(const IMagicMatcherList &matchers);

    friend QDebug operator<<(QDebug d, const QMimeType &mt);

    static QString formatFilterString(const QString &description,
                                      const QList<MimeGlobPattern> &globs);

private:
    explicit QMimeType(const QMimeTypeData &d);

    friend class BaseMimeTypeParser;
    friend class QMimeDatabasePrivate;
    QSharedDataPointer<QMimeTypeData> m_d;
};

QT_END_NAMESPACE

#endif // QMIMETYPE_H
