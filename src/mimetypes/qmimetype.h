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

#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QFileInfo;

class QMimeTypeData;

class QMIME_EXPORT QMimeType
{
public:
    QMimeType();
    QMimeType(const QMimeType &other);
    explicit QMimeType(const QMimeTypeData &dd);
    QMimeType &operator=(const QMimeType &other);
    ~QMimeType();

    bool operator==(const QMimeType &other) const;
    inline bool operator!=(const QMimeType &other) const
    { return !operator==(other); }

    void clear();

    bool isValid() const;

    const QString &name() const;

    const QStringList &aliases() const;

    const QString &comment() const;

    QString localeComment(const QString &locale = QString()) const;

    // TODO do not return const refs! This prevents any kind of non-trivial implementation

    const QString &genericIconName() const;

    const QString &iconName() const;

    const QStringList &globPatterns() const;

    const QStringList &parentMimeTypes() const;

    const QStringList &allParentMimeTypes() const;

    const QStringList &suffixes() const;
    const QString &preferredSuffix() const;

    bool inherits(const QString &mimeTypeName) const;

#if 0   // Seems unused
    bool matchesName(const QString &name) const;
#endif
    unsigned matchesData(const QByteArray &data) const;
    //unsigned matchesFile(QIODevice *device, const QString &fileName) const;
    //unsigned matchesFileBySuffix(const QString &fileName) const;

    QString filterString() const;

protected:
    friend class BaseMimeTypeParser;
    friend class MimeTypeMapEntry;
    friend class QMimeDatabasePrivate;
    friend class QMimeDatabase;
    friend class QMimeTypeData;

    QExplicitlySharedDataPointer<QMimeTypeData> d;
};

QT_END_NAMESPACE

#endif // QMIMETYPE_H
