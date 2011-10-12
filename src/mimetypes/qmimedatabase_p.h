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

// MimeMapEntry: Entry of a type map, consisting of type.
struct MimeMapEntry
{
    inline MimeMapEntry(const QMimeType &aType = QMimeType()) :
        type(aType)
    {}
    QMimeType type;
};

struct QMimeDatabasePrivate
{
    Q_DISABLE_COPY(QMimeDatabasePrivate)

    QMimeDatabasePrivate();
    ~QMimeDatabasePrivate();

    QMimeProviderBase *provider();
    void setProvider(QMimeProviderBase *theProvider);

    bool addMimeType(const QMimeType &mt);
    //void addGlobPattern(const QMimeGlobPattern &glob);

    QStringList filterStrings() const;

#if 0
    QList<QMimeGlobPattern> globPatterns() const;
    void setGlobPatterns(const QString &typeOrAlias, const QStringList &globPatterns);

    QStringList suffixes() const;
    bool setPreferredSuffix(const QString &typeOrAlias, const QString &suffix);

    void setMagicMatchers(const QString &typeOrAlias,
                          const QList<QMimeMagicRuleMatcher> &matchers);
#endif

    QList<QMimeType> mimeTypes() const;

#if 0
    static QList<QMimeGlobPattern> toGlobPatterns(const QStringList &patterns, const QString &mimeType,
                                                  int weight = QMimeGlobPattern::MaxWeight);
    static QStringList fromGlobPatterns(const QList<QMimeGlobPattern> &globPatterns);
#endif

    typedef QHash<QString, MimeMapEntry *> TypeMimeTypeMap;
#if 0 // This parentChildrenMap seems to be unused?
    typedef QMultiHash<QString, QString> ParentChildrenMap;
#endif
    typedef QHash<QString, QString> AliasMap;

    QMimeType findByType(const QString &type);
    QMimeType findByNameAndData(const QString &fileName, QIODevice *device, unsigned *priorityPtr);
    QMimeType findByData(const QByteArray &data, unsigned *priorityPtr);
    QStringList findByName(const QString &fileName);
    inline QString resolveAlias(const QString &name) const
    { return aliasMap.value(name, name); }

    TypeMimeTypeMap typeMimeTypeMap;
    AliasMap aliasMap;
#if 0 // This parentChildrenMap seems to be unused?
    ParentChildrenMap parentChildrenMap;
#endif
    mutable QMimeProviderBase *m_provider;
    QMutex mutex;
};


#if 0
class QMIME_EXPORT QMimeDatabaseBuilder
{
    Q_DISABLE_COPY(QMimeDatabaseBuilder)

public:
    QMimeDatabaseBuilder(QMimeDatabase *mimeDatabase);
    ~QMimeDatabaseBuilder();

    bool addMimeType(const QMimeType &mt);

    QStringList suffixes() const;
    bool setPreferredSuffix(const QString &typeOrAlias, const QString &suffix);
    QString preferredSuffixByType(const QString &type) const;
    QString preferredSuffixByNameAndData(const QString &fileName, QIODevice *device) const;

    QStringList globPatterns() const;
    void setGlobPatterns(const QString &typeOrAlias, const QStringList &globPatterns);

    void setMagicMatchers(const QString &typeOrAlias,
                          const QList<QMimeMagicRuleMatcher> &matchers);

    static QList<QMimeGlobPattern> toGlobPatterns(const QStringList &patterns,
                                                  const QString &mimeType,
                                                  int weight = QMimeGlobPattern::MaxWeight);
    static QStringList fromGlobPatterns(const QList<QMimeGlobPattern> &globPatterns);

private:
    QMimeDatabasePrivate *const d;
};
#endif

QT_END_NAMESPACE

#endif // QMIMEDATABASE_P_H
