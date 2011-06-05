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

#include "qmimemagicrule.h"

#include <QtCore/QStringList>
#include <qendian.h>

QT_BEGIN_NAMESPACE

// in the same order as Type!
static const char magicRuleTypes_string[] =
    "unknown\0"
    "string\0"
    "byte\0"
    "big16\0"
    "big32\0"
    "little16\0"
    "little32\0"
    "host16\0"
    "host32\0"
    "\0";

static const int magicRuleTypes_indices[] = {
    0, 8, 15, 20, 26, 32, 41, 50, 57, 64, 0
};

QMimeMagicRule::Type QMimeMagicRule::type(const QByteArray &type)
{
    for (int i = String; i <= Host32; ++i) {
        if (type == magicRuleTypes_string + magicRuleTypes_indices[i])
            return static_cast<Type>(i);
    }
    return Invalid;
}

QByteArray QMimeMagicRule::typeName(QMimeMagicRule::Type type)
{
    return magicRuleTypes_string + magicRuleTypes_indices[type];
}


struct QMimeMagicRulePrivate
{
    QMimeMagicRule::Type type;
    QByteArray matchValue;
    int startPos;
    int endPos;

    QByteArray pattern;
    quint32 number;

    typedef bool (*MatchFunction)(QMimeMagicRulePrivate *d, const QByteArray &data);
    MatchFunction matchFunction;
};


static bool matchBytes(QMimeMagicRulePrivate *d, const QByteArray &data)
{
    if (d->startPos >= data.size() || d->startPos + d->pattern.size() >= data.size())
        return false;

    // most common case: some string at position 0
    if (d->startPos == 0 && d->startPos == d->endPos)
        return data.startsWith(d->pattern);

    int end = d->endPos - d->startPos + d->pattern.size();
    if (end > data.size())
        end = data.size();

    return QByteArray::fromRawData(data.constData() + d->startPos, end).contains(d->pattern);
}

template <typename T>
static bool matchNumber(QMimeMagicRulePrivate *d, const QByteArray &data)
{
    const T value(d->number);

    const char *p = data.constData() + d->startPos;
    const char *e = p + qMin(data.size() - int(sizeof(T)), d->endPos + 1);
    while (p < e) {
        if (*reinterpret_cast<const T*>(p) == value)
            return true;
        ++p;
    }

    return false;
}

static inline QByteArray bytesFromSequence(const QByteArray &sequence)
{
    QByteArray result;

    bool ok;

    if (!sequence.startsWith('\\')) {
        char c = char(sequence.toInt(&ok, 0)); // autodetect
        if (ok)
            result += c;
        return result;
    }

    // Expect hex format value like \0x7f\0x45\0x4c\0x46
    foreach (const QByteArray &byte, sequence.split('\\')) {
        char c = char(byte.toInt(&ok, 16)); // hex
        if (!ok)
            return QByteArray();
        result += c;
    }

    return result;
}

QMimeMagicRule::QMimeMagicRule(QMimeMagicRule::Type type, const QByteArray &matchValue, int startPos, int endPos)
    : d(new QMimeMagicRulePrivate)
{
    uint value = 0;
    if (type >= Big16 && type <= Host32) {
        bool ok;
        value = matchValue.toUInt(&ok, 0); // autodetect
        if (!ok) {
            qWarning("QMimeMagicRule(): Can't convert \"%s\" to int", matchValue.constData());
            type = Invalid;
        }
    }

    d->type = type;
    d->matchValue = matchValue;
    d->startPos = startPos;
    d->endPos = endPos;

    switch (d->type) {
    case String:
        d->pattern = matchValue;
        d->matchFunction = matchBytes;
        break;
    case Byte:
        d->pattern = bytesFromSequence(matchValue);
        if (!d->pattern.isEmpty()) {
            d->matchFunction = matchBytes;
        } else {
            qWarning("QMimeMagicRule(): Incorrect byte sequence \"%s\"", matchValue.constData());
            d->matchFunction = 0;
        }
        break;
    case Big16:
    case Host16: // reverse on little-endians
        d->number = qFromBigEndian<quint16>(value);
        d->matchFunction = matchNumber<quint16>;
        break;
    case Little16:
        d->number = qFromLittleEndian<quint16>(value);
        d->matchFunction = matchNumber<quint16>;
        break;
    case Big32:
    case Host32: // reverse on little-endians
        d->number = qFromBigEndian<quint32>(value);
        d->matchFunction = matchNumber<quint32>;
        break;
    case Little32:
        d->number = qFromLittleEndian<quint32>(value);
        d->matchFunction = matchNumber<quint32>;
        break;
    default:
        d->matchFunction = 0;
    }
}

QMimeMagicRule::QMimeMagicRule(const QMimeMagicRule &other)
    : d(new QMimeMagicRulePrivate(*other.d))
{
}

QMimeMagicRule::~QMimeMagicRule()
{
}

QMimeMagicRule& QMimeMagicRule::operator=(const QMimeMagicRule &other)
{
    *d = *other.d;
    return *this;
}

QMimeMagicRule::Type QMimeMagicRule::type() const
{
    return d->type;
}

QByteArray QMimeMagicRule::matchValue() const
{
    return d->matchValue;
}

int QMimeMagicRule::startPos() const
{
    return d->startPos;
}

int QMimeMagicRule::endPos() const
{
    return d->endPos;
}

bool QMimeMagicRule::matches(const QByteArray &data) const
{
    return d->matchFunction && d->matchFunction(d.data(), data);
}

QT_END_NAMESPACE
