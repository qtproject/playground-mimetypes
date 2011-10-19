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

#include "qmimemagicrule_p.h"

#include <QtCore/QList>
#include <qendian.h>

QT_BEGIN_NAMESPACE

// in the same order as Type!
static const char magicRuleTypes_string[] =
    "invalid\0"
    "string\0"
    "host16\0"
    "host32\0"
    "big16\0"
    "big32\0"
    "little16\0"
    "little32\0"
    "byte\0"
    "\0";

static const int magicRuleTypes_indices[] = {
    0, 8, 15, 22, 29, 35, 41, 50, 59, 65, 0
};

QMimeMagicRule::Type QMimeMagicRule::type(const QByteArray &theTypeName)
{
    for (int i = String; i <= Byte; ++i) {
        if (theTypeName == magicRuleTypes_string + magicRuleTypes_indices[i])
            return static_cast<Type>(i);
    }
    return Invalid;
}

QByteArray QMimeMagicRule::typeName(QMimeMagicRule::Type theType)
{
    return magicRuleTypes_string + magicRuleTypes_indices[theType];
}

struct QMimeMagicRulePrivate
{
    bool operator==(const QMimeMagicRulePrivate &other) const;

    QMimeMagicRule::Type type;
    QByteArray value;
    int startPos;
    int endPos;
    QByteArray mask;

    QByteArray pattern;
    quint32 number;
    quint32 numberMask;

    typedef bool (*MatchFunction)(QMimeMagicRulePrivate *d, const QByteArray &data);
    MatchFunction matchFunction;
};

bool QMimeMagicRulePrivate::operator==(const QMimeMagicRulePrivate &other) const
{
    return type == other.type &&
           value == other.value &&
           startPos == other.startPos &&
           endPos == other.endPos &&
           mask == other.mask &&
           pattern == other.pattern &&
           number == other.number &&
           numberMask == other.numberMask &&
           matchFunction == other.matchFunction;
}

static bool matchString(QMimeMagicRulePrivate *d, const QByteArray &data)
{
    const char *p = data.constData() + d->startPos;
    const char *e = data.constData() + qMin(data.size() - d->pattern.size(), d->endPos + 1);
    for ( ; p < e; ++p) {
        int i = 0;
        while (i < d->pattern.size() && (p[i] & d->mask.at(i)) == d->pattern.at(i))
            ++i;
        if (i == d->pattern.size())
            return true;
    }

    return false;
}

template <typename T>
static bool matchNumber(QMimeMagicRulePrivate *d, const QByteArray &data)
{
    const T value(d->number);
    const T mask(d->numberMask);

    const char *p = data.constData() + d->startPos;
    const char *e = data.constData() + qMin(data.size() - int(sizeof(T)), d->endPos + 1);
    for ( ; p < e; ++p) {
        if ((*reinterpret_cast<const T*>(p) & mask) == value)
            return true;
    }

    return false;
}

static inline QByteArray makePattern(const QByteArray &value)
{
    QByteArray pattern(value.size(), Qt::Uninitialized);
    char *data = pattern.data();

    const char *p = value.constData();
    const char *e = p + value.size();
    for ( ; p < e; ++p) {
        if (*p == '\\' && ++p < e) {
            if (*p == 'x') { // hex (\\xff)
                char c = 0;
                for (int i = 0; i < 2 && p + 1 < e; ++i) {
                    ++p;
                    if (*p >= '0' && *p <= '9')
                        c = (c << 4) + *p - '0';
                    else if (*p >= 'a' && *p <= 'f')
                        c = (c << 4) + *p - 'a' + 10;
                    else if (*p >= 'A' && *p <= 'F')
                        c = (c << 4) + *p - 'A' + 10;
                    else
                        continue;
                }
                *data++ = c;
            } else if (*p >= '0' && *p <= '7') { // oct (\\7, or \\77, or \\377)
                char c = *p - '0';
                if (p + 1 < e && p[1] >= '0' && p[1] <= '7') {
                    c = (c << 3) + *(++p) - '0';
                    if (p + 1 < e && p[1] >= '0' && p[1] <= '7' && p[-1] <= '3')
                        c = (c << 3) + *(++p) - '0';
                }
                *data++ = c;
            } else { // escaped
                *data++ = *p;
            }
        } else {
            *data++ = *p;
        }
    }
    pattern.truncate(data - pattern.data());

    return pattern;
}

QMimeMagicRule::QMimeMagicRule(QMimeMagicRule::Type theType,
                               const QByteArray &theValue,
                               int theStartPos,
                               int theEndPos,
                               const QByteArray &theMask) :
    d(new QMimeMagicRulePrivate)
{
    Q_ASSERT(!theValue.isEmpty());

    d->type = theType;
    d->value = theValue;
    d->startPos = theStartPos;
    d->endPos = theEndPos;
    d->mask = theMask;
    d->matchFunction = 0;

    if (d->type >= Host16 && d->type <= Byte) {
        bool ok;
        d->number = d->value.toUInt(&ok, 0); // autodetect
        Q_ASSERT(ok);
        d->numberMask = !d->mask.isEmpty() ? d->mask.toUInt(&ok, 0) : 0; // autodetect
    }

    switch (d->type) {
    case String:
        d->pattern = makePattern(d->value);
        d->pattern.squeeze();
        if (!d->mask.isEmpty()) {
            Q_ASSERT(d->mask.size() >= 4 && d->mask.startsWith("0x"));
            d->mask = QByteArray::fromHex(QByteArray::fromRawData(d->mask.constData() + 2, d->mask.size() - 2));
            Q_ASSERT(d->mask.size() == d->pattern.size());

            // apply mask to pattern
            const char *m = d->mask.constData();
            char *p = d->pattern.data();
            const char *e = p + d->pattern.size();
            while (p < e)
                *p++ &= *m++;
        } else {
            d->mask.fill(static_cast<char>(0xff), d->pattern.size());
        }
        d->mask.squeeze();
        d->matchFunction = matchString;
        break;
    case Byte:
        if (d->number <= quint8(-1)) {
            if (d->numberMask == 0)
                d->numberMask = quint8(-1);
            d->matchFunction = matchNumber<quint8>;
        }
        break;
    case Big16:
    case Host16:
    case Little16:
        if (d->number <= quint16(-1)) {
            d->number = d->type == Little16 ? qFromLittleEndian<quint16>(d->number) : qFromBigEndian<quint16>(d->number);
            if (d->numberMask == 0)
                d->numberMask = quint16(-1);
            d->matchFunction = matchNumber<quint16>;
        }
        break;
    case Big32:
    case Host32:
    case Little32:
        if (d->number <= quint32(-1)) {
            d->number = d->type == Little32 ? qFromLittleEndian<quint32>(d->number) : qFromBigEndian<quint32>(d->number);
            if (d->numberMask == 0)
                d->numberMask = quint32(-1);
            d->matchFunction = matchNumber<quint32>;
        }
        break;
    default:
        break;
    }
}

QMimeMagicRule::QMimeMagicRule(const QMimeMagicRule &other) :
    d(new QMimeMagicRulePrivate(*other.d))
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

bool QMimeMagicRule::operator==(const QMimeMagicRule &other) const
{
    return d == other.d ||
           *d == *other.d;
}

QMimeMagicRule::Type QMimeMagicRule::type() const
{
    return d->type;
}

QByteArray QMimeMagicRule::value() const
{
    return d->value;
}

int QMimeMagicRule::startPos() const
{
    return d->startPos;
}

int QMimeMagicRule::endPos() const
{
    return d->endPos;
}

QByteArray QMimeMagicRule::mask() const
{
    QByteArray result = d->mask;
    if (d->type == String) {
        // restore '0x'
        result = "0x" + result.toHex();
    }
    return result;
}

bool QMimeMagicRule::isValid() const
{
    return d->matchFunction;
}

bool QMimeMagicRule::matches(const QByteArray &data) const
{
    return d->matchFunction && d->matchFunction(d.data(), data);
}

QT_END_NAMESPACE
