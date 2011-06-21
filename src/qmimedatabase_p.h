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

public:
    QMimeDatabasePrivate();
    ~QMimeDatabasePrivate();

    bool addMimeTypes(const QString &fileName, QString *errorMessage);
    bool addMimeTypes(QIODevice *device, QString *errorMessage);
    bool addMimeType(QMimeType mt);

    QStringList filterStrings() const;

    QStringList suffixes() const;

    QList<QMimeGlobPattern> globPatterns() const;

    QList<QMimeMagicRuleMatcher> magicMatchers() const;

    QList<QMimeType> mimeTypes() const;

    void syncUserModifiedMimeTypes();
    static QList<QMimeType> readUserModifiedMimeTypes();
    static void writeUserModifiedMimeTypes(const QList<QMimeType> &mimeTypes);
    void clearUserModifiedMimeTypes();

    static QList<QMimeGlobPattern> toGlobPatterns(const QStringList &patterns,
                                                  int weight = QMimeGlobPattern::DefaultWeight);
    static QStringList fromGlobPatterns(const QList<QMimeGlobPattern> &globPatterns);

private:
    typedef QHash<QString, QString> AliasMap;
    typedef QMultiHash<QString, QString> ParentChildrenMap;

    static const QString kModifiedMimeTypesFile;
    static QString kModifiedMimeTypesPath;

    bool addMimeTypes(QIODevice *device, const QString &fileName, QString *errorMessage);
    inline QString resolveAlias(const QString &name) const
    { return aliasMap.value(name, name); }

    QMimeType findByType(const QString &type) const;
    QMimeType findByFile(const QFileInfo &f, unsigned *priorityPtr) const;
    QMimeType findByData(const QByteArray &data, unsigned *priorityPtr) const;
    QMimeType findByName(const QString &name, unsigned *priorityPtr) const;

    void determineLevels();
    void raiseLevelRecursion(MimeMapEntry &e, int level);

    QHash<QString, MimeMapEntry*> typeMimeTypeMap;
    AliasMap aliasMap;
    ParentChildrenMap parentChildrenMap;
    int maxLevel;
    QMutex mutex;
};

QT_END_NAMESPACE

#endif // QMIMEDATABASE_P_H
