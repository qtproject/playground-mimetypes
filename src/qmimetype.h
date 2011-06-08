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

#include <QtCore/QRegExp>
#include <QtCore/QStringList>

#include "magicmatcher.h"

QT_BEGIN_NAMESPACE

class QFileInfo;

class QMIME_EXPORT QMimeGlobPattern
{
public:
    static const unsigned MaxWeight = 100;
    static const unsigned DefaultWeight = 50;
    static const unsigned MinWeight = 1;

    explicit QMimeGlobPattern(const QRegExp &regExp, unsigned weight = DefaultWeight) :
        m_regExp(regExp), m_weight(weight) {}
    ~QMimeGlobPattern() {}

    inline const QRegExp &regExp() const
    { return m_regExp; }
    inline unsigned weight() const
    { return m_weight; }

private:
    QRegExp m_regExp;
    int m_weight;
};

class QMimeTypeData;
class QMIME_EXPORT QMimeType
{
public:
    QMimeType();
    explicit QMimeType(const QString &type);
    QMimeType(const QString &type,
              const QList<QMimeMagicRuleMatcher> &matchers,
              const QList<QMimeGlobPattern> &globPatterns = QList<QMimeGlobPattern>(),
              const QStringList &subClassOf = QStringList());
    QMimeType(const QMimeType &other);
    ~QMimeType();

    QMimeType &operator=(const QMimeType &other);

    void clear();

    bool isValid() const;
    bool isTopLevel() const;

    QString type() const;
    void setType(const QString &type);

    QStringList aliases() const;
    void setAliases(const QStringList &aliases);

    QString comment() const;
    void setComment(const QString &comment);

    QString localeComment(const QString &locale = QString() /* en, de...*/) const;
    void setLocaleComment(const QString &locale, const QString &comment);

    QString genericIconName() const;
    void setGenericIconName(const QString &genericIconName);

    QList<QMimeGlobPattern> globPatterns() const;
    void setGlobPatterns(const QList<QMimeGlobPattern> &globPatterns);

    QList<QMimeMagicRuleMatcher> magicMatchers() const;
    void addMagicMatcher(const QMimeMagicRuleMatcher &matcher);
    void setMagicMatchers(const QList<QMimeMagicRuleMatcher> &matchers);

    QStringList subClassOf() const;
    void setSubClassOf(const QStringList &subClassOf);

    // Extension over standard mime data
    QStringList suffixes() const;
    QString preferredSuffix() const;
    bool setPreferredSuffix(const QString &preferredSuffix);

    bool matchesType(const QString &type) const;
    unsigned matchesData(const QByteArray &data) const;
    unsigned matchesFile(const QFileInfo &file) const;
    unsigned matchesName(const QString &name) const;

    QString filterString() const;

private:
    explicit QMimeType(const QMimeTypeData &dd);

    friend class BaseMimeTypeParser;
    friend class QMimeDatabasePrivate;

    QSharedDataPointer<QMimeTypeData> d;
};

QT_END_NAMESPACE

#endif // QMIMETYPE_H
