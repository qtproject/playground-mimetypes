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

#ifndef QMIMEPROVIDER_P_H
#define QMIMEPROVIDER_P_H

#include "qmimedatabase_p.h"

class QMimeProviderBase
{
public:
    QMimeProviderBase(QMimeDatabasePrivate *db);

    virtual bool isValid() = 0;
    virtual void ensureTypesLoaded() = 0;
    virtual void ensureGlobsLoaded() = 0;
    virtual void ensureMagicLoaded() = 0;

    QMimeDatabasePrivate* m_db;
};

/*
   Parses mime.cache (possibly on demand, for its different areas)
 */
class QMimeBinaryProvider : public QMimeProviderBase
{
public:
    QMimeBinaryProvider(QMimeDatabasePrivate *db);

    virtual bool isValid();
    virtual void ensureTypesLoaded();
    virtual void ensureGlobsLoaded();
    virtual void ensureMagicLoaded();

private:
    const QStringList m_cacheFiles;
};

/*
   Parses the raw XML files (slower)
 */
class QMimeXMLProvider : public QMimeProviderBase
{
public:
    QMimeXMLProvider(QMimeDatabasePrivate *db);

    virtual bool isValid();
    virtual void ensureTypesLoaded();
    virtual void ensureGlobsLoaded();
    virtual void ensureMagicLoaded();

    bool load(const QString &fileName, QString *errorMessage);

private:
    void ensureLoaded();
    void load(const QString &fileName);

    bool m_loaded;
};

#endif // QMIMEPROVIDER_P_H
