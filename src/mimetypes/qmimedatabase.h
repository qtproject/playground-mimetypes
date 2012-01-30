/****************************************************************************
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
****************************************************************************/

#ifndef QMIMEDATABASE_H_INCLUDED
#define QMIMEDATABASE_H_INCLUDED

#include "qmime_global.h"

#include "qmimetype.h"

#include <QtCore/QStringList>

// ------------------------------------------------------------------------------------------------

QT_BEGIN_NAMESPACE

class QByteArray;
class QFileInfo;
class QIODevice;
class QUrl;

struct QMimeDatabasePrivate;
class QMIME_EXPORT QMimeDatabase
{
    Q_DISABLE_COPY(QMimeDatabase)

public:
    QMimeDatabase();
    ~QMimeDatabase();

    QMimeType mimeTypeForName(const QString &nameOrAlias) const;

    enum MatchFlags {
        MatchDefault = 0x0,
        MatchExtension = 0x1,
        MatchContent = 0x2
    };

    QMimeType mimeTypeForFile(const QString &fileName, MatchFlags flags = MatchDefault) const;
    QMimeType mimeTypeForFile(const QFileInfo &fileInfo, MatchFlags flags = MatchDefault) const;
    QList<QMimeType> mimeTypesForFileName(const QString &fileName) const;

    QMimeType mimeTypeForData(const QByteArray &data) const;
    QMimeType mimeTypeForData(QIODevice *device) const;

    QMimeType mimeTypeForUrl(const QUrl &url) const;
    QMimeType mimeTypeForNameAndData(const QString &fileName, QIODevice *device) const;
    QMimeType mimeTypeForNameAndData(const QString &fileName, const QByteArray &data) const;

    QString suffixForFileName(const QString &fileName) const;

    QList<QMimeType> allMimeTypes() const;

    QMimeDatabasePrivate *data_ptr() { return d; }

private:
    QMimeDatabasePrivate *d;
};

QT_END_NAMESPACE

#endif   // QMIMEDATABASE_H_INCLUDED
