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

class FileMatchContext;

class QMimeTypeData : public QSharedData
{
public:
    typedef QHash<QString, QString> LocaleHash;

    QMimeTypeData();

    void clear();
    void assignSuffix(const QString &pattern);
    void assignSuffixes(const QStringList &patterns);
#ifndef QT_NO_DEBUG_STREAM
    void debug(QTextStream &str, int indent = 0) const;
#endif

    unsigned matchesFileBySuffix(const QString &name) const;
    unsigned matchesData(const QByteArray &data) const;

    static QString formatFilterString(const QString &description,
                                      const QList<QMimeGlobPattern> &globs);

    const QRegExp suffixPattern;

    QString type;
    QString comment;
    LocaleHash localeComments;
    QStringList aliases;
    QList<QMimeGlobPattern> globPatterns;
    QStringList subClassOf;
    QString preferredSuffix;
    QStringList suffixes;
    IMagicMatcherList magicMatchers;
};

enum { debugMimeDB = 0 };

QT_END_NAMESPACE

#endif // QMIMETYPE_P_H
