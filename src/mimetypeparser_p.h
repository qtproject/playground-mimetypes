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

#ifndef MIMETYPEPARSER_P_H
#define MIMETYPEPARSER_P_H

#include "qmimedatabase_p.h"

QT_BEGIN_NAMESPACE

class QIODevice;

class BaseMimeTypeParser
{
    Q_DISABLE_COPY(BaseMimeTypeParser)

public:
    BaseMimeTypeParser() {}
    virtual ~BaseMimeTypeParser() {}

    bool parse(QIODevice *dev, const QString &fileName, QString *errorMessage);

protected:
    virtual bool process(const QMimeType &t, QString *errorMessage) = 0;
    virtual bool process(const QMimeGlobPattern &t, QString *errorMessage) = 0;

private:
    enum ParseState {
        ParseBeginning,
        ParseMimeInfo,
        ParseMimeType,
        ParseComment,
        ParseGenericIcon,
        ParseGlobPattern,
        ParseSubClass,
        ParseAlias,
        ParseMagic,
        ParseMagicMatchRule,
        ParseOtherMimeTypeSubTag,
        ParseError
    };

    static ParseState nextState(ParseState currentState, const QStringRef &startElement);
};


class MimeTypeParser : public BaseMimeTypeParser
{
public:
    explicit MimeTypeParser(QMimeDatabasePrivate &db) : m_db(db) {}

protected:
    inline bool process(const QMimeType &t, QString *)
    { m_db.addMimeType(t); return true; }

    inline bool process(const QMimeGlobPattern &glob, QString *)
    { m_db.addGlobPattern(glob); return true; }

private:
    QMimeDatabasePrivate &m_db;
};

QT_END_NAMESPACE

#endif // MIMETYPEPARSER_P_H
