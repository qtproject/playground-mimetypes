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

#include "qmimeprovider_p.h"

#include "mimetypeparser_p.h"
#include <qstandardpaths.h>
#include <QFile>
#include <QDebug>

QMimeProviderBase::QMimeProviderBase(QMimeDatabasePrivate *db)
    : m_db(db)
{
}

QMimeBinaryProvider::QMimeBinaryProvider(QMimeDatabasePrivate *db)
    : QMimeProviderBase(db)
{
}

bool QMimeBinaryProvider::isValid()
{
    const QString globalCacheFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("mime/mime.cache"));
    if (!globalCacheFile.isEmpty()) {
        const QString localCacheFile = QStandardPaths::storageLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/mime/mime.cache");
        qDebug() << globalCacheFile << localCacheFile;
        return globalCacheFile != localCacheFile;
    }
    return false;
}

void QMimeBinaryProvider::ensureTypesLoaded()
{
}

void QMimeBinaryProvider::ensureGlobsLoaded()
{
}

void QMimeBinaryProvider::ensureMagicLoaded()
{
}

QMimeXMLProvider::QMimeXMLProvider(QMimeDatabasePrivate *db)
    : QMimeProviderBase(db), m_loaded(false)
{
}

bool QMimeXMLProvider::isValid()
{
    return true;
}

void QMimeXMLProvider::ensureTypesLoaded()
{
    ensureLoaded();
}

void QMimeXMLProvider::ensureGlobsLoaded()
{
    ensureLoaded();
}

void QMimeXMLProvider::ensureMagicLoaded()
{
    ensureLoaded();
}

void QMimeXMLProvider::ensureLoaded()
{
    if (!m_loaded) {
        // TODO  look for freedesktop.org.xml file in the system
        //       if not found, use Qt's own copy

        // TODO: putting the xml file in the resource is a hack for now
        // We should instead install the file as part of installing Qt

        // TODO and then we also need to locate user-modified mimetypes, using QStandardPaths::storageLocation() + QDir::entryList
        const QString fileName = QLatin1String(":/qmime/freedesktop.org.xml");
        load(fileName);
    }
}

void QMimeXMLProvider::load(const QString &fileName)
{
    QString errorMessage;
    if (!load(fileName, &errorMessage))
        qWarning("QMimeDatabase: Error loading %s\n%s", qPrintable(fileName), qPrintable(errorMessage));
}

bool QMimeXMLProvider::load(const QString &fileName, QString *errorMessage)
{
    m_loaded = true;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage)
            *errorMessage = QString::fromLatin1("Cannot open %1: %2").arg(fileName, file.errorString());
        return false;
    }

    if (errorMessage)
        errorMessage->clear();

    MimeTypeParser parser(*m_db);
    return parser.parse(&file, fileName, errorMessage);
}
