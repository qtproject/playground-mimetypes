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

QT_BEGIN_NAMESPACE

#define MIN_MATCH_WEIGHT 50

// MimeMapEntry: Entry of a type map, consisting of type and level.
struct MimeMapEntry
{
    static const int Dangling = 32767;

    inline MimeMapEntry(const QMimeType &aType = QMimeType(), int aLevel = Dangling) :
        type(aType), level(aLevel)
    {}

    QMimeType type;
    int level; // hierachy level
};


class QMimeDatabasePrivate
{
    Q_DISABLE_COPY(QMimeDatabasePrivate)
    friend class QMimeDatabase;
    friend class QMimeDatabaseBuilder;

public:
    QMimeDatabasePrivate();
    ~QMimeDatabasePrivate();

    bool addMimeTypes(const QString &fileName, QString *errorMessage);
    bool addMimeTypes(QIODevice *device, QString *errorMessage);
    bool addMimeType(const QMimeType &mt);

    QStringList filterStrings() const;

    QList<QMimeType> mimeTypes() const;

private:
    typedef QHash<QString, QString> AliasMap;
    typedef QMultiHash<QString, QString> ParentChildrenMap;

    bool addMimeTypes(QIODevice *device, const QString &fileName, QString *errorMessage);
    inline QString resolveAlias(const QString &name) const
    { return aliasMap.value(name, name); }

    QMimeType findByType(const QString &type) const;
    QMimeType findByFile(const QFileInfo &f, unsigned *priorityPtr) const;
    QMimeType findByData(const QByteArray &data, unsigned *priorityPtr) const;
    QMimeType findByName(const QString &name, unsigned *priorityPtr) const;

    unsigned matchesBySuffix(const QMimeType &type, const QString &name, unsigned *length) const;

    void determineLevels();
    void raiseLevelRecursion(MimeMapEntry &e, int level);

    QHash<QString, MimeMapEntry*> typeMimeTypeMap;
    AliasMap aliasMap;
    ParentChildrenMap parentChildrenMap;
    int maxLevel;
    QMutex mutex;
};


class QMimeDatabaseBuilder
{
    Q_DISABLE_COPY(QMimeDatabaseBuilder)
public:
    QMimeDatabaseBuilder();
    ~QMimeDatabaseBuilder();

    bool addMimeTypes(const QString &fileName, QString *errorMessage);
    bool addMimeTypes(QIODevice *device, QString *errorMessage);

private:
    QMimeDatabasePrivate *const d;
};

QT_END_NAMESPACE

#endif // QMIMEDATABASE_P_H
