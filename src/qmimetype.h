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

#include "qmime_global.h"

#include "magicmatcher.h"

#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QFileInfo;

class QMIME_EXPORT QMimeGlobPattern
{
public:
    static const unsigned MaxWeight = 100;
    static const unsigned MinWeight = 1;

    explicit QMimeGlobPattern(const QRegExp &regExp, unsigned weight = MaxWeight);
    ~QMimeGlobPattern();

    const QRegExp &regExp() const;
    unsigned weight() const;

private:
    QRegExp m_regExp;
    int m_weight;
};

class QMimeTypeData;
class QMIME_EXPORT QMimeType
{
public:
    typedef IMagicMatcher::IMagicMatcherList IMagicMatcherList;
    typedef IMagicMatcher::IMagicMatcherSharedPointer IMagicMatcherSharedPointer;

    QMimeType();
    explicit QMimeType(const QString &type);
    QMimeType(const QString &type,
              const IMagicMatcherList &matchers,
              const QList<QMimeGlobPattern> &globPatterns = QList<QMimeGlobPattern>(),
              const QStringList &subClassOf = QStringList());
    QMimeType(const QMimeType&);
    QMimeType &operator=(const QMimeType&);
    ~QMimeType();

    void clear();

    bool isValid() const;
    bool isTopLevel() const;

    QString type() const;
    void setType(const QString &type);

    QStringList aliases() const;
    void setAliases(const QStringList &);

    QString comment() const;
    void setComment(const QString &comment);

    QString localeComment(const QString &locale = QString() /* en, de...*/) const;
    void setLocaleComment(const QString &locale, const QString &comment);

    QList<QMimeGlobPattern> globPatterns() const;
    void setGlobPatterns(const QList<QMimeGlobPattern> &);

    QStringList subClassOf() const;
    void setSubClassOf(const QStringList &);

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

private:
    explicit QMimeType(const QMimeTypeData &d);

    friend class BaseMimeTypeParser;
    friend class QMimeDatabasePrivate;

    QSharedDataPointer<QMimeTypeData> m_d;
};

QT_END_NAMESPACE

#endif // QMIMETYPE_H
