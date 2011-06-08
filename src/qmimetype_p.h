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

#include "qmimetype.h"

#include <QtCore/QHash>

QT_BEGIN_NAMESPACE

class QTextStream;

class QMimeTypeData : public QSharedData
{
public:
    typedef QHash<QString, QString> LocaleHash;

    QMimeTypeData();

    void clear();
    void assignSuffix(const QString &pattern);
    void assignSuffixes(const QStringList &patterns);

    unsigned matchesFileBySuffix(const QString &name) const;
    unsigned matchesData(const QByteArray &data) const;

    const QRegExp suffixPattern;

    QString type;
    QString comment;
    LocaleHash localeComments;
    QStringList aliases;
    QString genericIconName;
    QList<QMimeGlobPattern> globPatterns;
    QStringList subClassOf;
    QString preferredSuffix;
    QStringList suffixes;
    QList<QMimeMagicRuleMatcher> magicMatchers;
};

QT_END_NAMESPACE

#endif // QMIMETYPE_P_H
