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

#ifndef QMIMEDATABASE_P_H
#define QMIMEDATABASE_P_H

#include <QtCore/QMultiHash>
#include <QtCore/QMutex>

#include "qmimetype.h"
#include "qmimetype_p.h"
#include "qmimeglobpattern_p.h"

QT_BEGIN_NAMESPACE

class QMimeDatabase;
class QMimeProviderBase;

// MimeTypeMapEntry: Entry of a MIME type map, consisting of a MIME type.
struct MimeTypeMapEntry
{
    inline MimeTypeMapEntry(const QMimeType &aType = QMimeType()) :
        type(aType)
    {}
    QMimeType type;
};

struct QMimeDatabasePrivate
{
    Q_DISABLE_COPY(QMimeDatabasePrivate)

    QMimeDatabasePrivate();
    ~QMimeDatabasePrivate();

    static QMimeDatabasePrivate* instance();

    QMimeProviderBase *provider();
    void setProvider(QMimeProviderBase *theProvider);

    bool addMimeType(const QMimeType &mt);

    QStringList filterStrings() const;

    bool inherits(const QString &mime, const QString &parent);

    QList<QMimeType> mimeTypes() const;

    typedef QHash<QString, MimeTypeMapEntry *> NameMimeTypeMap;
    typedef QHash<QString, QString> AliasMap;

    QMimeType mimeTypeForName(const QString &nameOrAlias);
    QMimeType findByNameAndData(const QString &fileName, QIODevice *device, unsigned *priorityPtr);
    QMimeType findByData(const QByteArray &data, unsigned *priorityPtr);
    QStringList findByName(const QString &fileName);
    inline QString resolveAlias(const QString &name) const
    { return aliasMap.value(name, name); }

    NameMimeTypeMap nameMimeTypeMap;
    AliasMap aliasMap;
    mutable QMimeProviderBase *m_provider;
    QMutex mutex;
};

QT_END_NAMESPACE

#endif // QMIMEDATABASE_P_H
