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

#include "magicmatcher.h"
#include "magicmatcher_p.h"

#include "qmimetype_p.h"

#include <QtCore/QDebug>
#include <qendian.h>

// UTF16 byte order marks
static const char bigEndianByteOrderMarkC[] = "\xFE\xFF";
static const char littleEndianByteOrderMarkC[] = "\xFF\xFE";

/*!
    \class IMagicMatcher

    \brief Interface for a Mime type magic matcher (examinig file contents).

    \sa MimeType, MimeDatabase, MagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa FileMatchContext, BinaryMatcher, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
*/

//namespace Core {

typedef QSharedPointer<MagicRuleMatcher> MagicRuleMatcherPtr;

//namespace Internal {

/*!
    \class FileMatchContext

    \brief Context passed on to the mime types when looking for a file match.

    It exists to enable reading the file contents "on demand"
    (as opposed to each mime type trying to open and read while checking).

    \sa MimeType, MimeDatabase, IMagicMatcher, MagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa BinaryMatcher, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
*/

FileMatchContext::FileMatchContext(const QFileInfo &fi) :
    m_fileInfo(fi),
    m_fileName(fi.fileName()),
    m_state(fi.isFile() && fi.isReadable() && fi.size() > 0 ? DataNotRead : NoDataAvailable)
{
}

QByteArray FileMatchContext::data()
{
    // Do we need to read?
    if (m_state == DataNotRead) {
        const QString fullName = m_fileInfo.absoluteFilePath();
        QFile file(fullName);
        if (file.open(QIODevice::ReadOnly)) {
            m_data = file.read(MaxData);
            m_state = DataRead;
        } else {
            qWarning("%s failed to open %s: %s\n", Q_FUNC_INFO, fullName.toUtf8().constData(), file.errorString().toUtf8().constData());
            m_state = NoDataAvailable;
        }
    }
    return m_data;
}

/*!
    \class BinaryMatcher
    \brief The binary fallback matcher for mime type "application/octet-stream".

    \sa MimeType, MimeDatabase, IMagicMatcher, MagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa FileMatchContext, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
*/

/*!
    \class HeuristicTextMagicMatcher
    \brief Heuristic text file matcher for mime types.

    If the data do not contain any character below tab (9), detect as text.
    Additionally, check on UTF16 byte order markers.

    \sa MimeType, MimeDatabase, IMagicMatcher, MagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa FileMatchContext, BinaryMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
*/

bool HeuristicTextMagicMatcher::isTextFile(const QByteArray &data)
{
    const int size = data.size();
    for (int i = 0; i < size; i++) {
        const char c = data.at(i);
        if (c >= 0x01 && c < 0x09) // Sure-fire binary
            return false;
        if (c == 0)             // Check for UTF16
            return data.startsWith(bigEndianByteOrderMarkC) || data.startsWith(littleEndianByteOrderMarkC);
    }
    return true;
}

bool HeuristicTextMagicMatcher::matches(const QByteArray &data) const
{
    const bool rc = isTextFile(data);
    if (debugMimeDB)
        qDebug() << Q_FUNC_INFO << " on " << data.size() << " returns " << rc;
    return rc;
}

//} // namespace Internal

/*!
    \class MagicRule
    \brief Base class for standard Magic match rules based on contents
    and offset specification.

    Stores the offset and provides conversion helpers.
    Base class for implementations for "string" and "byte".
    (Others like little16, big16, etc. can be created whenever there is a need.)

    \sa MimeType, MimeDatabase, IMagicMatcher, MagicRuleMatcher, MagicStringRule, MagicByteRule, GlobPattern
    \sa FileMatchContext, BinaryMatcher, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
 */

const QChar MagicRule::kColon(QLatin1Char(':'));

MagicRule::MagicRule(int startPos, int endPos) : m_startPos(startPos), m_endPos(endPos)
{
}

MagicRule::~MagicRule()
{
}

int MagicRule::startPos() const
{
    return m_startPos;
}

int MagicRule::endPos() const
{
    return m_endPos;
}

QString MagicRule::toOffset(const QPair<int, int> &startEnd)
{
    return QString(QLatin1String("%1:%2")).arg(startEnd.first).arg(startEnd.second);
}

QPair<int, int> MagicRule::fromOffset(const QString &offset)
{
    const QStringList &startEnd = offset.split(kColon);
    Q_ASSERT(startEnd.size() == 2);
    return qMakePair(startEnd.at(0).toInt(), startEnd.at(1).toInt());
}

/*!
    \class MagicStringRule
    \brief Match on a string.

    \sa MimeType, MimeDatabase, IMagicMatcher, MagicRuleMatcher, MagicRule, MagicByteRule, GlobPattern
    \sa FileMatchContext, BinaryMatcher, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
 */

const QString MagicStringRule::kMatchType("string");

MagicStringRule::MagicStringRule(const QString &s, int startPos, int endPos) :
    MagicRule(startPos, endPos), m_pattern(s.toUtf8())
{
}

MagicStringRule::~MagicStringRule()
{
}

QString MagicStringRule::matchType() const
{
    return kMatchType;
}

QString MagicStringRule::matchValue() const
{
    return m_pattern;
}

bool MagicStringRule::matches(const QByteArray &data) const
{
    // Quick check
    if ((startPos() + m_pattern.size()) > data.size())
        return false;
    // Most common: some string at position 0:
    if (startPos() == 0 && startPos() == endPos())
        return data.startsWith(m_pattern);
    // Range
    if (data.size() <= startPos())
        return false;

    int end = endPos() - startPos() + m_pattern.size();
    return QByteArray::fromRawData(data.data() + startPos(), qMin(end, data.size())).contains(m_pattern);
}

/*!
    \class MagicByteRule
    \brief Match on a sequence of binary data.

    Format:
    \code
    \0x7f\0x45\0x4c\0x46
    \endcode

    \sa MimeType, MimeDatabase, IMagicMatcher, MagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa FileMatchContext, BinaryMatcher, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
 */

const QString MagicByteRule::kMatchType(QLatin1String("byte"));

MagicByteRule::MagicByteRule(const QString &s, int startPos, int endPos) :
    MagicRule(startPos, endPos), m_bytesSize(0)
{
    if (validateByteSequence(s, &m_bytes))
        m_bytesSize = m_bytes.size();
    else
        m_bytes.clear();
}

MagicByteRule::~MagicByteRule()
{
}

bool MagicByteRule::validateByteSequence(const QString &sequence, QList<int> *bytes)
{
    // Expect an hex format value like this: \0x7f\0x45\0x4c\0x46
    const QStringList &byteSequence = sequence.split(QLatin1Char('\\'), QString::SkipEmptyParts);
    foreach (const QString &byte, byteSequence) {
        bool ok;
        const int hex = byte.toInt(&ok, 16);
        if (ok) {
            if (bytes)
                bytes->push_back(hex);
        } else {
            return false;
        }
    }
    return true;
}

QString MagicByteRule::matchType() const
{
    return kMatchType;
}

QString MagicByteRule::matchValue() const
{
    QString value;
    foreach (int byte, m_bytes)
        value.append(QString(QLatin1String("\\0x%1")).arg(byte, 0, 16));
    return value;
}

bool MagicByteRule::matches(const QByteArray &data) const
{
    if (m_bytesSize == 0)
        return false;

    const int dataSize = data.size();
    for (int start = startPos(); start <= endPos(); ++start) {
        if ((start + m_bytesSize) > dataSize)
            return false;

        int matchAt = 0;
        while (matchAt < m_bytesSize) {
            if (data.at(start + matchAt) != m_bytes.at(matchAt))
                break;
            ++matchAt;
        }
        if (matchAt == m_bytesSize)
            return true;
    }

    return false;
}

MagicNumberRule::MagicNumberRule(const QString &s, int startPos, int endPos,
                                 Size size, Endianness endianness) :
    MagicRule(startPos, endPos),
    m_stringValue(s),
    m_size(size),
    m_endiannes(endianness)
{
    bool ok = false;
    uint value = 0;

    if (!s.isEmpty()) {

        if (s[0] == '0') {
            if (s.size() > 1 && s[1] == 'x')
                value = s.toUInt(&ok, 16);
            else
                value = s.toUInt(&ok, 8);
        } else
            value = s.toUInt(&ok, 10);

    }

    if (!ok)
        qWarning() << QString("MagicNumberRule::MagicNumberRule: Can't convert %1 string to int").arg(s);

    if (size == Size16) {

        if (endianness == LittleEndian)
            m_value16 = qFromLittleEndian<quint16>(value);
        else if (endianness == BigEndian)
            m_value16 = qFromBigEndian<quint16>(value);
        else
            m_value16 = qFromBigEndian<quint16>(value); // reverse on little-endians

    } else if (size == Size32) {

        if (endianness == LittleEndian)
             m_value32 = qFromLittleEndian<quint32>(value);
        else if (endianness == BigEndian)
            m_value32 = qFromBigEndian<quint32>(value);
        else
            m_value32 = qFromBigEndian<quint32>(value); // reverse on little-endians

    }
}

MagicNumberRule::~MagicNumberRule()
{
}

QString MagicNumberRule::matchType() const
{
    static const QString kBig16(QLatin1String("big16"));
    static const QString kBig32(QLatin1String("big32"));
    static const QString kLittle16(QLatin1String("little16"));
    static const QString kLittle32(QLatin1String("little32"));
    static const QString kHost16(QLatin1String("host16"));
    static const QString kHost32(QLatin1String("host32"));

    if (m_size == Size16) {
        if (m_endiannes == BigEndian)
            return kBig16;
        else if (m_endiannes == LittleEndian)
            return kLittle16;
        else
            return kHost16;
    } else {
        if (m_endiannes == BigEndian)
            return kBig32;
        else if (m_endiannes == LittleEndian)
            return kLittle32;
        else
            return kHost32;
    }
}

QString MagicNumberRule::matchValue() const
{
    return m_stringValue;
}

bool MagicNumberRule::matches(const QByteArray &data) const
{
    if (m_size == Size16) {

        for (int i = startPos(); i <= endPos(); i++) {
            if (data.size() < i + 2)
                return false;

            if ( *(reinterpret_cast<const quint16*>(data.data() + i)) == m_value16 )
                return true;
        }

    } else if (m_size == Size32) {

        for (int i = startPos(); i <= endPos(); i++) {
            if (data.size() < i + 4)
                return false;

            if ( *(reinterpret_cast<const quint32*>(data.data() + i)) == m_value32 )
                return true;
        }

    }

    return false;
}

/*!
    \class MagicRuleMatcher

    \brief A Magic matcher that checks a number of rules based on operator "or".

    It is used for rules parsed from XML files.

    \sa MimeType, MimeDatabase, IMagicMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa FileMatchContext, BinaryMatcher, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
*/

MagicRuleMatcher::MagicRuleMatcher() :
     m_priority(65535)
{
}

void MagicRuleMatcher::add(const MagicRuleSharedPointer &rule)
{
    m_list.append(rule);
}

void MagicRuleMatcher::add(const MagicRuleList &ruleList)
{
    m_list.append(ruleList);
}

MagicRuleMatcher::MagicRuleList MagicRuleMatcher::magicRules() const
{
    return m_list;
}

bool MagicRuleMatcher::matches(const QByteArray &data) const
{
    const MagicRuleList::const_iterator cend = m_list.constEnd();
    for (MagicRuleList::const_iterator it = m_list.constBegin(); it != cend; ++it)
        if ( (*it)->matches(data))
            return true;
    return false;
}

int MagicRuleMatcher::priority() const
{
    return m_priority;
}

void MagicRuleMatcher::setPriority(int p)
{
    m_priority = p;
}

IMagicMatcher::IMagicMatcherList MagicRuleMatcher::createMatchers(
    const QHash<int, MagicRuleList> &rulesByPriority)
{
    IMagicMatcher::IMagicMatcherList matchers;
    QHash<int, MagicRuleList>::const_iterator ruleIt = rulesByPriority.begin();
    for (; ruleIt != rulesByPriority.end(); ++ruleIt) {
        MagicRuleMatcher *magicRuleMatcher = new MagicRuleMatcher();
        magicRuleMatcher->setPriority(ruleIt.key());
        magicRuleMatcher->add(ruleIt.value());
        matchers.append(IMagicMatcher::IMagicMatcherSharedPointer(magicRuleMatcher));
    }
    return matchers;
}
