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

#include "qmime_global.h"

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>

#include "qmimemagicrule.h"

QT_BEGIN_NAMESPACE

class QMIME_EXPORT IMagicMatcher
{
    Q_DISABLE_COPY(IMagicMatcher)
protected:
    IMagicMatcher() {}
public:
    virtual ~IMagicMatcher() {}

    enum Type { RuleMatcher, CustomMatcher };
    virtual Type type() const { return CustomMatcher; }

    // Check for a match on contents of a file
    virtual bool matches(const QByteArray &data) const = 0;
    // Return a priority value from 1..100
    virtual unsigned priority() const = 0;
};

typedef QSharedPointer<IMagicMatcher> IMagicMatcherSharedPointer;
typedef QList<IMagicMatcherSharedPointer> IMagicMatcherList;


class QMIME_EXPORT MagicRuleMatcher : public IMagicMatcher
{
    Q_DISABLE_COPY(MagicRuleMatcher)
public:
    MagicRuleMatcher();

    void addRule(const QMimeMagicRule &rule);
    void addRules(const QList<QMimeMagicRule> &rules);
    QList<QMimeMagicRule> magicRules() const;

    virtual Type type() const;

    virtual bool matches(const QByteArray &data) const;

    virtual unsigned priority() const;
    void setPriority(unsigned priority);

private:
    QList<QMimeMagicRule> m_list;
    unsigned m_priority;
};

QT_END_NAMESPACE

#endif // MAGICMATCHER_H
