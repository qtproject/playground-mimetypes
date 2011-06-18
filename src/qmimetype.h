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

class QMimeTypeData;
class QMimeGlobPattern;
class QMIME_EXPORT QMimeType
{
public:
    QMimeType(const QMimeType &other);
    ~QMimeType();

    QMimeType &operator=(const QMimeType &other);

    bool operator==(const QMimeType &other) const;
    inline bool operator!=(const QMimeType &other) const
    { return operator==(other); }

    void clear();

    bool isValid() const;

    QString type() const;

    QStringList aliases() const;

    QString comment() const;

    QString localeComment(const QString &locale = QString() /* en, de...*/) const;

    QString genericIconName() const;

    QStringList globPatterns() const;
    QList<QMimeGlobPattern> weightedGlobPatterns() const;

    QList<QMimeMagicRuleMatcher> magicMatchers() const;

    QStringList subClassOf() const;

    // Extension over standard mime data
    QStringList suffixes() const;
    QString preferredSuffix() const;

    bool matchesType(const QString &type) const;
    unsigned matchesData(const QByteArray &data) const;
    unsigned matchesFile(const QFileInfo &file) const;
    unsigned matchesName(const QString &name) const;

    QString filterString() const;

protected:
    QMimeType();
    explicit QMimeType(const QMimeTypeData &dd);

    friend class BaseMimeTypeParser;
    friend class MimeMapEntry;
    friend class QMimeDatabasePrivate;

    QExplicitlySharedDataPointer<QMimeTypeData> d;
};

class QMIME_EXPORT QMutableMimeType : public QMimeType
{
public:
    QMutableMimeType();
    explicit QMutableMimeType(const QString &type,
                              const QList<QMimeMagicRuleMatcher> &matchers =
                              QList<QMimeMagicRuleMatcher>(),
                              const QList<QMimeGlobPattern> &globPatterns = QList<QMimeGlobPattern>(),
                              const QStringList &subClassOf = QStringList());
    QMutableMimeType(const QMimeType &other);

    QMutableMimeType(const QMutableMimeType &other);
    ~QMutableMimeType();

    void setType(const QString &type);

    void setAliases(const QStringList &aliases);

    void setComment(const QString &comment);

    void setLocaleComment(const QString &locale, const QString &comment);

    void setGenericIconName(const QString &genericIconName);

    void setWeightedGlobPatterns(const QList<QMimeGlobPattern> &globPatterns);

    void addMagicMatcher(const QMimeMagicRuleMatcher &matcher);
    void setMagicMatchers(const QList<QMimeMagicRuleMatcher> &matchers);

    void setSubClassOf(const QStringList &subClassOf);

    bool setPreferredSuffix(const QString &preferredSuffix);

    QSharedDataPointer<QMimeTypeData> d;
};

QT_END_NAMESPACE

#endif // QMIMETYPE_H
