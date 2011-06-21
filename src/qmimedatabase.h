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
    ~QMimeDatabase();

    bool addMimeType(const QMimeType &mt);
    bool addMimeTypes(const QString &fileName, QString *errorMessage);
    bool addMimeTypes(QIODevice *device, QString *errorMessage);

    QMimeType findByType(const QString &type) const;
    QMimeType findByFile(const QFileInfo &fileInfo) const;
    QMimeType findByName(const QString &name) const;
    QMimeType findByData(const QByteArray &data) const;

    QList<QMimeType> mimeTypes() const;

    QList<QMimeGlobPattern> globPatterns() const;

    QList<QMimeMagicRuleMatcher> magicMatchers() const;

    // Return all known suffixes
    QStringList suffixes() const;
    QString preferredSuffixByType(const QString &type) const;
    QString preferredSuffixByFile(const QFileInfo &f) const;

    QStringList filterStrings() const;
    // Return a string with all the possible file filters, for use with file dialogs
    QString allFiltersString(QString *allFilesFilter = 0) const;

    // The mime types from the functions bellow are considered only in regard to
    // their glob patterns and rule-based magic matchers.
    void syncUserModifiedMimeTypes();
    void clearUserModifiedMimeTypes();
    static QList<QMimeType> readUserModifiedMimeTypes();
    static void writeUserModifiedMimeTypes(const QList<QMimeType> &mimeTypes);

    static QList<QMimeGlobPattern> toGlobPatterns(const QStringList &patterns,
                                                  int weight = QMimeGlobPattern::DefaultWeight);
    static QStringList fromGlobPatterns(const QList<QMimeGlobPattern> &globPatterns);

private:
    QMimeDatabasePrivate *const d;
};

QT_END_NAMESPACE

#endif // QMIMEDATABASE_H
