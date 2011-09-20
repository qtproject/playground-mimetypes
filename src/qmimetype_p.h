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

class QMimeGlobPattern
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


class QMimeTypeData : public QSharedData
{
public:
    typedef QHash<QString, QString> LocaleHash;

    QMimeTypeData();

    void clear();
    void assignSuffix(const QString &pattern);
    void assignSuffixes(const QStringList &patterns);

    unsigned matchesFileBySuffix(const QString &name) const;
    unsigned matchesData(const QByteArray &data) const;

    void setType(const QString &type);

    void setWeightedGlobPatterns(const QList<QMimeGlobPattern> &globPatterns);

    void setMagicMatchers(const QList<QMimeMagicRuleMatcher> &matchers);

    const QRegExp suffixPattern;

    static QList<QMimeGlobPattern> toGlobPatterns(const QStringList &patterns,
                                                  int weight = QMimeGlobPattern::DefaultWeight);
    static QStringList fromGlobPatterns(const QList<QMimeGlobPattern> &globPatterns);

    QString type;
    QString comment;
    LocaleHash localeComments;
    QStringList aliases;
    QString genericIconName;
    QList<QMimeGlobPattern> globPatterns;
    QStringList subClassOf;
    QString preferredSuffix;
    QStringList suffixes;
    QList<QMimeMagicRuleMatcher> magicMatchers;
};

QT_END_NAMESPACE

#endif // QMIMETYPE_P_H
