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

void MagicRuleMatcher::add(const QMimeMagicRule &rule)
{
    m_list.append(rule);
}

void MagicRuleMatcher::add(const QMimeMagicRuleList &ruleList)
{
    m_list.append(ruleList);
}

QMimeMagicRuleList MagicRuleMatcher::magicRules() const
{
    return m_list;
}

IMagicMatcher::Type MagicRuleMatcher::type() const
{
    return RuleMatcher;
}

bool MagicRuleMatcher::matches(const QByteArray &data) const
{
    if (m_list.isEmpty())
        return false;
//    qDebug() << this->magicRules();
    for (int i = 0; i < m_list.size(); i++)
        if ( !m_list.at(i).matches(data))
            return false;
    return true;
}

unsigned MagicRuleMatcher::priority() const
{
    return m_priority;
}

void MagicRuleMatcher::setPriority(unsigned p)
{
    m_priority = p;
}

IMagicMatcher::IMagicMatcherList MagicRuleMatcher::createMatchers(
    const QHash<int, QMimeMagicRuleList > &rulesByPriority)
{
    IMagicMatcher::IMagicMatcherList matchers;
    QHash<int, QMimeMagicRuleList>::const_iterator ruleIt = rulesByPriority.begin();
    for (; ruleIt != rulesByPriority.end(); ++ruleIt) {
        MagicRuleMatcher *magicRuleMatcher = new MagicRuleMatcher();
        magicRuleMatcher->setPriority(ruleIt.key());
        magicRuleMatcher->add(ruleIt.value());
        matchers.append(IMagicMatcher::IMagicMatcherSharedPointer(magicRuleMatcher));
    }
    return matchers;
}
