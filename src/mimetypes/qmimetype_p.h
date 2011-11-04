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

#ifndef QMIMETYPE_P_H_INCLUDED
#define QMIMETYPE_P_H_INCLUDED

#include "qmimetype.h"

#include <QtCore/QHash>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

struct QMIME_EXPORT QMimeTypePrivate : public QSharedData
{
    typedef QHash<QString, QString> LocaleHash;

    QMimeTypePrivate();
    explicit QMimeTypePrivate(const QMimeType &other);

    void clear();

    bool operator==(const QMimeTypePrivate &other) const;

    void addGlobPattern(const QString &pattern);

    //unsigned matchesFileBySuffix(const QString &fileName) const;
    //unsigned matchesData(const QByteArray &data) const;

    QString name;
    QStringList aliases;
    QString comment;
    LocaleHash localeComments;
    QString genericIconName;
    QString iconName;
    QStringList globPatterns;
};

QT_END_NAMESPACE

#endif   // QMIMETYPE_P_H_INCLUDED
