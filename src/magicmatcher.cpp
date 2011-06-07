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

QT_BEGIN_NAMESPACE

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

typedef QSharedPointer<QMimeMagicRuleMatcher> MagicRuleMatcherPtr;

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
        QFile file(m_fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly)) {
            m_data = file.read(MaxData);
            m_state = DataRead;
        } else {
            qWarning("%s failed to open %s: %s", Q_FUNC_INFO,
                     m_fileInfo.absoluteFilePath().toLocal8Bit().constData(),
                     file.errorString().toLocal8Bit().constData());
            m_state = NoDataAvailable;
        }
    }
    return m_data;
}

/*!
    \class MagicRuleMatcher

    \brief A Magic matcher that checks a number of rules based on operator "or".

    It is used for rules parsed from XML files.

    \sa MimeType, MimeDatabase, IMagicMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa FileMatchContext, BinaryMatcher, HeuristicTextMagicMatcher
    \sa BaseMimeTypeParser, MimeTypeParser
*/

QMimeMagicRuleMatcher::QMimeMagicRuleMatcher(unsigned priority)
    : m_priority(priority)
{
}

void QMimeMagicRuleMatcher::addRule(const QMimeMagicRule &rule)
{
    m_list.append(rule);
}

void QMimeMagicRuleMatcher::addRules(const QList<QMimeMagicRule> &rules)
{
    m_list.append(rules);
}

QList<QMimeMagicRule> QMimeMagicRuleMatcher::magicRules() const
{
    return m_list;
}

bool QMimeMagicRuleMatcher::matches(const QByteArray &data) const
{
    if (m_list.isEmpty())
        return false;

    foreach (const QMimeMagicRule &magicRule, m_list) {
        if (!magicRule.matches(data))
            return false;
    }

    return true;
}

unsigned QMimeMagicRuleMatcher::priority() const
{
    return m_priority;
}

void QMimeMagicRuleMatcher::setPriority(unsigned priority)
{
    m_priority = priority;
}

QT_END_NAMESPACE
