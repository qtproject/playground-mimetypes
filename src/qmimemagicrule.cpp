#include "qmimemagicrule.h"

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <qendian.h>

QT_BEGIN_NAMESPACE

static const char *kStrings[] = { "unknown", "string", "byte", "big16", "big32",
                                   "little16", "little32", "host16", "host32"};

struct QMimeMagicRulePrivate
{
    QMimeMagicRule::Type type;
    QString value;
    int startPos;
    int endPos;

    QByteArray pattern; // String
    quint16 value16; // *16
    quint32 value32; // *32

    typedef bool (*MatchFunction)(QMimeMagicRulePrivate* d, const QByteArray &data);

    MatchFunction matchFunction;
};

static bool matchBytes(QMimeMagicRulePrivate* d, const QByteArray& data)
{
    // Quick check
    if ((d->startPos + d->pattern.size()) > data.size())
        return false;
    // Most common: some string at position 0:
    if (d->startPos == 0 && d->startPos == d->endPos)
        return data.startsWith(d->pattern);
    // Range
    if (data.size() <= d->startPos)
        return false;

    int end = d->endPos - d->startPos + d->pattern.size();
    return QByteArray::fromRawData(data.constData() + d->startPos,
                                   qMin(end, data.size())).contains(d->pattern);
}

static bool match16(QMimeMagicRulePrivate *d, const QByteArray &data)
{
    const char *p = data.constData() + d->startPos;
    const char *e = data.constData() + qMin(data.size() - 2, d->endPos + 1);
    while (p < e) {
        if (*reinterpret_cast<const quint16*>(p) == d->value16)
            return true;
        ++p;
    }
    return false;
}

static bool match32(QMimeMagicRulePrivate *d, const QByteArray &data)
{
    const char *p = data.constData() + d->startPos;
    const char *e = data.constData() + qMin(data.size() - 4, d->endPos + 1);
    while (p < e) {
        if (*reinterpret_cast<const quint32*>(p) == d->value32)
            return true;
        ++p;
    }
    return false;
}

static bool matchInvalid(QMimeMagicRulePrivate *, const QByteArray &)
{
    return false;
}

QByteArray bytesFromSequence(const QString &sequence)
{
    QByteArray result;

    bool ok;
    if (!sequence.startsWith(QLatin1Char('\\'))) {
        char c = sequence.toInt(&ok, 0); // autodetect
        if (ok)
            result += c;
        return result;
    }

    // Expect hex format value like this: \0x7f\0x45\0x4c\0x46
    foreach (const QString &byte, sequence.split(QLatin1Char('\\'))) {
        result += char(byte.toInt(&ok, 16));
        if (!ok)
            return QByteArray();
    }

    return result;
}

QMimeMagicRule::QMimeMagicRule(Type type, const QString &string, int startPos, int endPos) :
    d(new QMimeMagicRulePrivate)
{
    d->type = type;
    d->value = string;
    d->startPos = startPos;
    d->endPos = endPos;

    uint value = 0;
    if (type >= Big16 && type <= Host32) {
        bool ok;
        value = string.toUInt(&ok, 0); // autodetect

        if (!ok)
            qWarning() << QString("QMimeMagicRule::QMimeMagicRule: Can't convert %1 string to int").
                          arg(string);
    }

    switch (type) {
    case String:
        d->pattern = string.toUtf8();
        d->matchFunction = matchBytes;
        break;
    case Byte:
        d->pattern = bytesFromSequence(string);
        if (!d->pattern.isEmpty())
            d->matchFunction = matchBytes;
        else {
            qWarning() << QString("QMimeMagicRule::QMimeMagicRule: Wrong byte sequence %1").
                          arg(string);
            d->matchFunction = matchInvalid;
        }
        break;
    case Big16:
    case Host16: // reverse on little-endians
        d->value16 = qFromBigEndian<quint16>(value);
        d->matchFunction = match16;
        break;
    case Little16:
        d->value16 = qFromLittleEndian<quint16>(value);
        d->matchFunction = match16;
        break;
    case Big32:
    case Host32: // reverse on little-endians
        d->value32 = qFromBigEndian<quint32>(value);
        d->matchFunction = match32;
        break;
    case Little32:
        d->value32 = qFromLittleEndian<quint32>(value);
        d->matchFunction = match32;
        break;
    default:
        d->matchFunction = matchInvalid;
    }
}

QMimeMagicRule::~QMimeMagicRule()
{
    delete d;
}

QMimeMagicRule::QMimeMagicRule(const QMimeMagicRule &other) :
    d(new QMimeMagicRulePrivate(*other.d))
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

QString QMimeMagicRule::matchType() const
{
    return QLatin1String(kStrings[d->type]);
}

QString QMimeMagicRule::matchValue() const
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

bool QMimeMagicRule::matches(const QByteArray &data) const
{
    return d->matchFunction(d, data);
}

QString QMimeMagicRule::toOffset(const QPair<int, int> &startEnd)
{
    return QString(QLatin1String("%1:%2")).arg(startEnd.first).arg(startEnd.second);
}

QPair<int, int> QMimeMagicRule::fromOffset(const QString &offset)
{
    const QStringList &startEnd = offset.split(QChar(QLatin1Char(':')));
    Q_ASSERT(startEnd.size() == 2);
    return qMakePair(startEnd.at(0).toInt(), startEnd.at(1).toInt());
}

QMimeMagicRule::Type QMimeMagicRule::stringToType(const QByteArray &type)
{
    for (int i = QMimeMagicRule::Invalid; i <= QMimeMagicRule::Host32; i++) {
        if (type == kStrings[i])
            return (QMimeMagicRule::Type)i;
    }
    return QMimeMagicRule::Invalid;
}

QT_END_NAMESPACE
