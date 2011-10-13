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

#ifndef QMIMETYPE_P_H
#define QMIMETYPE_P_H

#include "magicmatcher.h"
#include "qmimetype.h"

#include <QtCore/QHash>

QT_BEGIN_NAMESPACE

class QMimeMagicRuleMatcher;

class QMIME_EXPORT QMimeTypeData : public QSharedData
{
public:
    typedef QHash<QString, QString> LocaleHash;

    QMimeTypeData();
    explicit QMimeTypeData(const QMimeType &other);

    void clear();
    void addGlobPattern(const QString &pattern);

    //unsigned matchesFileBySuffix(const QString &fileName) const;
    unsigned matchesData(const QByteArray &data) const;

    const QRegExp suffixPattern;

    QString name;
    QString comment;
    LocaleHash localeComments;
    QStringList aliases;
    QString genericIconName;
    QStringList globPatterns;
    QStringList subClassOf;
    QString preferredSuffix;
    QStringList suffixes;
    QList<QMimeMagicRuleMatcher> magicMatchers;
};

QT_END_NAMESPACE

#endif // QMIMETYPE_P_H
