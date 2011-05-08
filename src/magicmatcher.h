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

#ifndef MAGICMATCHER_H
#define MAGICMATCHER_H

#include "qmimetype_global.h"

#include <QtCore/QByteArray>
#include <QtCore/QPair>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QMIMETYPE_EXPORT IMagicMatcher
{
    Q_DISABLE_COPY(IMagicMatcher)
protected:
    IMagicMatcher() {}
public:
    typedef QSharedPointer<IMagicMatcher> IMagicMatcherSharedPointer;
    typedef QList<IMagicMatcherSharedPointer> IMagicMatcherList;

    // Check for a match on contents of a file
    virtual bool matches(const QByteArray &data) const = 0;
    // Return a priority value from 1..100
    virtual int priority() const = 0;
    virtual ~IMagicMatcher() {}
};

class QMIMETYPE_EXPORT MagicRule
{
    Q_DISABLE_COPY(MagicRule)
public:
    MagicRule(int startPos, int endPos);
    virtual ~MagicRule();

    virtual QString matchType() const = 0;
    virtual QString matchValue() const = 0;
    virtual bool matches(const QByteArray &data) const = 0;

    int startPos() const;
    int endPos() const;

    static QString toOffset(const QPair<int, int> &startEnd);
    static QPair<int, int> fromOffset(const QString &offset);

private:
    static const QChar kColon;

    const int m_startPos;
    const int m_endPos;
};

class QMIMETYPE_EXPORT MagicStringRule : public MagicRule
{
public:
    MagicStringRule(const QString &s, int startPos, int endPos);
    virtual ~MagicStringRule();

    virtual QString matchType() const;
    virtual QString matchValue() const;
    virtual bool matches(const QByteArray &data) const;

    static const QString kMatchType;

private:
    const QByteArray m_pattern;
};

class QMIMETYPE_EXPORT MagicByteRule : public MagicRule
{
public:
    MagicByteRule(const QString &s, int startPos, int endPos);
    virtual ~MagicByteRule();

    virtual QString matchType() const;
    virtual QString matchValue() const;
    virtual bool matches(const QByteArray &data) const;

    static bool validateByteSequence(const QString &sequence, QList<int> *bytes = 0);

    static const QString kMatchType;

private:
    int m_bytesSize;
    QList<int> m_bytes;
};

class QMIMETYPE_EXPORT MagicRuleMatcher : public IMagicMatcher
{
    Q_DISABLE_COPY(MagicRuleMatcher)
public:
    typedef QSharedPointer<MagicRule> MagicRuleSharedPointer;
    typedef QList<MagicRuleSharedPointer> MagicRuleList;

    MagicRuleMatcher();

    void add(const MagicRuleSharedPointer &rule);
    void add(const MagicRuleList &ruleList);
    MagicRuleList magicRules() const;

    virtual bool matches(const QByteArray &data) const;

    virtual int priority() const;
    void setPriority(int p);

    // Create a list of MagicRuleMatchers from a hash of rules indexed by priorities.
    static IMagicMatcher::IMagicMatcherList createMatchers(const QHash<int, MagicRuleList> &);

private:
    MagicRuleList m_list;
    int m_priority;
};

QT_END_NAMESPACE

#endif // MAGICMATCHER_H
