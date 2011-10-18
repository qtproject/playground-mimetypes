/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtMimeTypes addon of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativemimetype_p.h"

#include "qmimetype_p.h"

#include <QtCore/QDebug>

// ------------------------------------------------------------------------------------------------

QDeclarativeMimeType::QDeclarativeMimeType(QObject *theParent) :
        QObject(theParent),
        m_MimeType()
{}

// ------------------------------------------------------------------------------------------------

QDeclarativeMimeType::QDeclarativeMimeType(const QMimeType &other, QObject *theParent) :
        QObject(theParent),
        m_MimeType(other)
{}

// ------------------------------------------------------------------------------------------------

QDeclarativeMimeType::~QDeclarativeMimeType()
{
    //qDebug() << Q_FUNC_INFO << "name():" << name();
}

// ------------------------------------------------------------------------------------------------

void QDeclarativeMimeType::assign(QDeclarativeMimeType *other)
{
    if (other == 0) {
        qWarning() << Q_FUNC_INFO << "other:" << other;
        return;
    }

    m_MimeType = other->m_MimeType;
}

// ------------------------------------------------------------------------------------------------

bool QDeclarativeMimeType::equals(QDeclarativeMimeType *other) const
{
    if (other == 0) {
        qWarning() << Q_FUNC_INFO << "other:" << other;
        return false;
    }

    return m_MimeType == other->m_MimeType;
}

// ------------------------------------------------------------------------------------------------

QMimeType QDeclarativeMimeType::mimeType() const
{
    return m_MimeType;
}

// ------------------------------------------------------------------------------------------------

bool QDeclarativeMimeType::isValid() const
{
    return m_MimeType.isValid();
}

// ------------------------------------------------------------------------------------------------

QString QDeclarativeMimeType::name() const
{
    return m_MimeType.name();
}

// ------------------------------------------------------------------------------------------------

static QMimeType buildMimeType (
                     const QString &name,
                     const QString &comment,
                     const QString &genericIconName,
                     const QString &iconName,
                     const QStringList &suffixes
                 )
{
    QMimeTypeData mimeTypeData;
    mimeTypeData.name = name;
    mimeTypeData.comment = comment;
    mimeTypeData.genericIconName = genericIconName;
    mimeTypeData.iconName = iconName;
    mimeTypeData.suffixes = suffixes;
    return QMimeType(mimeTypeData);
}

// ------------------------------------------------------------------------------------------------

void QDeclarativeMimeType::setName(const QString &newName)
{
    m_MimeType = buildMimeType(newName, m_MimeType.comment(), m_MimeType.genericIconName(), m_MimeType.iconName(), m_MimeType.suffixes());
}

// ------------------------------------------------------------------------------------------------

QString QDeclarativeMimeType::comment() const
{
    return m_MimeType.comment();
}

// ------------------------------------------------------------------------------------------------

void QDeclarativeMimeType::setComment(const QString &newComment)
{
    m_MimeType = buildMimeType(m_MimeType.name(), newComment, m_MimeType.genericIconName(), m_MimeType.iconName(), m_MimeType.suffixes());
}

// ------------------------------------------------------------------------------------------------

QString QDeclarativeMimeType::genericIconName() const
{
    return m_MimeType.genericIconName();
}

// ------------------------------------------------------------------------------------------------

void QDeclarativeMimeType::setGenericIconName(const QString &newGenericIconName)
{
    m_MimeType = buildMimeType(m_MimeType.name(), m_MimeType.comment(), newGenericIconName, m_MimeType.iconName(), m_MimeType.suffixes());
}

// ------------------------------------------------------------------------------------------------

QString QDeclarativeMimeType::iconName() const
{
    return m_MimeType.iconName();
}

// ------------------------------------------------------------------------------------------------

void QDeclarativeMimeType::setIconName(const QString &newIconName)
{
    m_MimeType = buildMimeType(m_MimeType.name(), m_MimeType.comment(), m_MimeType.genericIconName(), newIconName, m_MimeType.suffixes());
}

// ------------------------------------------------------------------------------------------------

QVariantList QDeclarativeMimeType::globPatterns() const
{
    QVariantList result;

    foreach (const QString &pattern, m_MimeType.globPatterns()) {
        result << pattern;
    }

    return result;
}

// ------------------------------------------------------------------------------------------------

QVariantList QDeclarativeMimeType::suffixes() const
{
    QVariantList result;

    foreach (const QString &suffix, m_MimeType.suffixes()) {
        result << suffix;
    }

    return result;
}

// ------------------------------------------------------------------------------------------------

void QDeclarativeMimeType::setSuffixes(const QVariantList &newSuffixes)
{
    QList<QString> newSuffixesStringList;

    foreach (const QVariant &newSuffix, newSuffixes) {
        if (newSuffix.type() != QVariant::String) {
            qWarning() << Q_FUNC_INFO << "newSuffix" << newSuffix << " is not a string!";
            continue;
        }

        newSuffixesStringList << newSuffix.toString();
    }

    m_MimeType = buildMimeType(m_MimeType.name(), m_MimeType.comment(), m_MimeType.genericIconName(), m_MimeType.iconName(), newSuffixesStringList);
}
