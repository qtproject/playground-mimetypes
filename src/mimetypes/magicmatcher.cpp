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

#include <qendian.h>

QT_BEGIN_NAMESPACE

// TODO (add to spec) UTF16 byte order marks
//static const char bigEndianByteOrderMarkC[] = "\xFE\xFF";
//static const char littleEndianByteOrderMarkC[] = "\xFF\xFE";

/*!
    \class FileMatchContext
    \internal

    \brief Context passed on to the MIME types when looking for a file match.

    It exists to enable reading the file contents "on demand"
    (as opposed to each MIME type trying to open and read while checking).

    \sa QMimeType, QMimeDatabase, QMimeMagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa BaseMimeTypeParser, MimeTypeParser
*/

FileMatchContext::FileMatchContext(QIODevice *device, const QString &fileName) :
    m_device(device),
    m_fileName(fileName),
    m_state(DataNotRead)
{
}

bool FileMatchContext::isReadable()
{
    if (m_state == DataNotRead) {
        if (m_device->open(QIODevice::ReadOnly)) {
            // TODO use an intermediate state "device open but data not read yet?"
            // Better - read data on demand?
            m_data = m_device->read(MaxData);
            m_state = DataRead;
        }
    }
    return m_state == DataRead;
}

QByteArray FileMatchContext::data()
{
    // Do we need to read?
    if (m_state == DataNotRead) {
        if (m_device->open(QIODevice::ReadOnly)) {
            m_data = m_device->read(MaxData);
            m_state = DataRead;
        } else {
            qWarning("%s failed to open %s: %s", Q_FUNC_INFO,
                     qPrintable(m_fileName),
                     qPrintable(m_device->errorString()));
            m_state = NoDataAvailable;
        }
    }
    return m_data;
}

/*!
    \class QMimeMagicRuleMatcher

    \brief A Magic matcher that checks a number of rules based on operator "or".

    It is used for rules parsed from XML files.

    \sa QMimeType, QMimeDatabase, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa BaseMimeTypeParser, MimeTypeParser
*/

QMimeMagicRuleMatcher::QMimeMagicRuleMatcher(unsigned priority) :
    m_priority(priority)
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

// Check for a match on contents of a file
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

// Return a priority value from 1..100
unsigned QMimeMagicRuleMatcher::priority() const
{
    return m_priority;
}

void QMimeMagicRuleMatcher::setPriority(unsigned priority)
{
    m_priority = priority;
}

QT_END_NAMESPACE
