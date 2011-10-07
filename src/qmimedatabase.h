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

#ifndef QMIMEDATABASE_H
#define QMIMEDATABASE_H

#include "qmime_global.h"

#include "qmimetype.h"

#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QByteArray;
class QFileInfo;
class QIODevice;

class QMimeDatabasePrivate;
class QMIME_EXPORT QMimeDatabase
{
    Q_DISABLE_COPY(QMimeDatabase)

public:
    QMimeDatabase();
    explicit QMimeDatabase(QMimeDatabasePrivate *theD);
    ~QMimeDatabase();

    QMimeType findByType(const QString &type) const;
    QMimeType findByFile(const QFileInfo &fileInfo) const;
    QMimeType findByName(const QString &name) const;
    QMimeType findByData(const QByteArray &data) const;

    QList<QMimeType> mimeTypes() const;

    QStringList filterStrings() const;
    QString allFiltersString(QString *allFilesFilter = 0) const;

private:
    QMimeDatabasePrivate *d;

public:
    QMimeDatabasePrivate *data_ptr() { return d; }
};

QT_END_NAMESPACE

#endif // QMIMEDATABASE_H
