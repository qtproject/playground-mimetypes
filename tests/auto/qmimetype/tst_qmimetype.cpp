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

#include "tst_qmimetype.h"

#include <qmimetype_p.h>

#include <QMimeType>

#include <QtTest/QtTest>

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::initTestCase()
{
}

// ------------------------------------------------------------------------------------------------

static const QString &pngMimeTypeName()
{
    static const QString result ("image/png");
    return result;
}

// ------------------------------------------------------------------------------------------------

static const QString &pngMimeTypeDisplayName()
{
    static const QString result ("PNG File");
    return result;
}

// ------------------------------------------------------------------------------------------------

static const QString &pngMimeTypeIconUrl()
{
    static const QString result ("/usr/share/icons/oxygen/64x64/mimetypes/image-x-generic.png");
    return result;
}

// ------------------------------------------------------------------------------------------------

static QStringList buildPngMimeTypeFilenameExtensions()
{
    QStringList result;
    result << ".png";
    return result;
}

// ------------------------------------------------------------------------------------------------

static const QStringList &pngMimeTypeFilenameExtensions()
{
    static const QStringList result (buildPngMimeTypeFilenameExtensions());
    return result;
}

// ------------------------------------------------------------------------------------------------

static QMimeTypeData buildMimeTypeData (
                         const QString &name,
                         const QString &comment,
                         const QString &genericIconName,
                         const QStringList &suffixes
                     )
{
    QMimeTypeData instantiatedPngMimeTypeData;
    instantiatedPngMimeTypeData.name = name;
    instantiatedPngMimeTypeData.comment = comment;
    instantiatedPngMimeTypeData.genericIconName = genericIconName;
    instantiatedPngMimeTypeData.suffixes = suffixes;
    return instantiatedPngMimeTypeData;
}

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::test_isValid()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
              );

    QVERIFY(instantiatedPngMimeType.isValid());

    QMimeType otherPngMimeType (instantiatedPngMimeType);

    QVERIFY(otherPngMimeType.isValid());
    QCOMPARE(instantiatedPngMimeType, otherPngMimeType);

    QMimeType defaultMimeType;

    QVERIFY(!defaultMimeType.isValid());
}

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::test_name()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
              );

    QMimeType otherPngMimeType (
                  buildMimeTypeData(QString(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
              );

    // Verify that the Id is part of the equality test:
    QCOMPARE(instantiatedPngMimeType.name(), pngMimeTypeName());

    QVERIFY(instantiatedPngMimeType != otherPngMimeType);
    QVERIFY(!(instantiatedPngMimeType == otherPngMimeType));
}

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::test_displayName()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
              );

    QMimeType otherPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), QString(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
              );

    // Verify that the DisplayName is part of the equality test:
    QCOMPARE(instantiatedPngMimeType.comment(), pngMimeTypeDisplayName());

    QVERIFY(instantiatedPngMimeType != otherPngMimeType);
    QVERIFY(!(instantiatedPngMimeType == otherPngMimeType));
}

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::test_iconUrl()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
              );

    QMimeType otherPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), QString(), pngMimeTypeFilenameExtensions())
              );

    // Verify that the IconUrl is part of the equality test:
    QCOMPARE(instantiatedPngMimeType.genericIconName(), pngMimeTypeIconUrl());

    QVERIFY(instantiatedPngMimeType != otherPngMimeType);
    QVERIFY(!(instantiatedPngMimeType == otherPngMimeType));
}

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::test_filenameExtensions()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
              );

    QMimeType otherPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), QStringList())
              );

    // Verify that the FilenameExtensions are part of the equality test:
    QCOMPARE(instantiatedPngMimeType.suffixes(), pngMimeTypeFilenameExtensions());

    QVERIFY(instantiatedPngMimeType != otherPngMimeType);
    QVERIFY(!(instantiatedPngMimeType == otherPngMimeType));
}

// ------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    tst_qmimetype tc;
    return QTest::qExec(&tc, argc, argv);
}
