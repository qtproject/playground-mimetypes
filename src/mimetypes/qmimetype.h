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

    bool isValid() const;

    bool isDefault() const;

    QString name() const;

    QStringList aliases() const;

    QString comment() const;

    QString localeComment(const QString &locale = QString()) const;

    QString genericIconName() const;

    QString iconName() const;

    QStringList globPatterns() const;

    QStringList parentMimeTypes() const;

    QStringList allParentMimeTypes() const;

    QStringList suffixes() const;
    QString preferredSuffix() const;

    bool inherits(const QString &mimeTypeName) const;

    QString filterString() const;

protected:
    friend class BaseMimeTypeParser;
    friend class MimeTypeMapEntry;
    friend class QMimeDatabasePrivate;
    friend class QMimeXMLProvider;
    friend class QMimeTypeData;

    QExplicitlySharedDataPointer<QMimeTypeData> d;
};

QT_END_NAMESPACE

#endif // QMIMETYPE_H
