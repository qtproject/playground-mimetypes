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

#ifndef QMIMEMAGICRULE_H
#define QMIMEMAGICRULE_H

#include "qmime_global.h"

#include <QtCore/QScopedPointer>

QT_BEGIN_NAMESPACE

class QMimeMagicRulePrivate;
class QMIME_EXPORT QMimeMagicRule
{
public:
    enum Type { Invalid = 0, String, Byte, Big16, Big32, Little16, Little32, Host16, Host32 };

    QMimeMagicRule(Type type, const QByteArray &matchValue, int startPos, int endPos);
    QMimeMagicRule(const QMimeMagicRule &other);
    ~QMimeMagicRule();

    QMimeMagicRule &operator=(const QMimeMagicRule &other);

    Type type() const;
    QByteArray matchValue() const;
    int startPos() const;
    int endPos() const;

    bool matches(const QByteArray &data) const;

    static Type type(const QByteArray &type);
    static QByteArray typeName(Type type);

private:
    const QScopedPointer<QMimeMagicRulePrivate> d;
};

QT_END_NAMESPACE

#endif // QMIMEMAGICRULE_H
