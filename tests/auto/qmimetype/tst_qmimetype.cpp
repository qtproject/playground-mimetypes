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

#include <qmimemagicrulematcher_p.h>
#include <qmimetype_p.h>

#include <QMimeType>
#include <QMimeDatabase>

#include <QtTest/QtTest>

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::initTestCase()
{
    qputenv("XDG_DATA_DIRS", "doesnotexist");
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
                         const QString &iconName,
                         const QStringList &suffixes
                    )
{
    QMimeTypeData instantiatedPngMimeTypeData;
    instantiatedPngMimeTypeData.name = name;
    instantiatedPngMimeTypeData.comment = comment;
    instantiatedPngMimeTypeData.genericIconName = genericIconName;
    instantiatedPngMimeTypeData.iconName = iconName;
    instantiatedPngMimeTypeData.suffixes = suffixes;
    return instantiatedPngMimeTypeData;
}

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::test_isValid()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
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
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
             );

    QMimeType otherPngMimeType (
                  buildMimeTypeData(QString(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
             );

    // Verify that the Id is part of the equality test:
    QCOMPARE(instantiatedPngMimeType.name(), pngMimeTypeName());

    QVERIFY(instantiatedPngMimeType != otherPngMimeType);
    QVERIFY(!(instantiatedPngMimeType == otherPngMimeType));
}

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::test_comment()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
             );

    QMimeType otherPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), QString(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
             );

    // Verify that the DisplayName is part of the equality test:
    QCOMPARE(instantiatedPngMimeType.comment(), pngMimeTypeDisplayName());

    QVERIFY(instantiatedPngMimeType != otherPngMimeType);
    QVERIFY(!(instantiatedPngMimeType == otherPngMimeType));
}

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::test_genericIconName()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
             );

    QMimeType otherPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), QString(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
             );

    // Verify that the IconUrl is part of the equality test:
    QCOMPARE(instantiatedPngMimeType.genericIconName(), pngMimeTypeIconUrl());

    QVERIFY(instantiatedPngMimeType != otherPngMimeType);
    QVERIFY(!(instantiatedPngMimeType == otherPngMimeType));
}

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::test_iconName()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
             );

    QMimeType otherPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), QString(), pngMimeTypeFilenameExtensions())
             );

    // Verify that the IconUrl is part of the equality test:
    QCOMPARE(instantiatedPngMimeType.iconName(), pngMimeTypeIconUrl());

    QVERIFY(instantiatedPngMimeType != otherPngMimeType);
    QVERIFY(!(instantiatedPngMimeType == otherPngMimeType));
}

// ------------------------------------------------------------------------------------------------

void tst_qmimetype::test_suffixes()
{
    QMimeType instantiatedPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), pngMimeTypeFilenameExtensions())
             );

    QMimeType otherPngMimeType (
                  buildMimeTypeData(pngMimeTypeName(), pngMimeTypeDisplayName(), pngMimeTypeIconUrl(), pngMimeTypeIconUrl(), QStringList())
             );

    // Verify that the FilenameExtensions are part of the equality test:
    QCOMPARE(instantiatedPngMimeType.suffixes(), pngMimeTypeFilenameExtensions());

    QVERIFY(instantiatedPngMimeType != otherPngMimeType);
    QVERIFY(!(instantiatedPngMimeType == otherPngMimeType));
}

void tst_qmimetype::test_inheritance()
{
    QMimeDatabase db;

    // All file-like mimetypes inherit from octet-stream
    const QMimeType wordperfect = db.mimeTypeForName("application/vnd.wordperfect");
    QVERIFY(wordperfect.isValid());
    QCOMPARE(wordperfect.parentMimeTypes().join(","), QString("application/octet-stream"));
    QVERIFY(wordperfect.inherits("application/octet-stream"));

    QVERIFY(db.mimeTypeForName("image/svg+xml-compressed").inherits("application/x-gzip"));

    // Check that msword derives from ole-storage
    const QMimeType msword = db.mimeTypeForName("application/msword");
    QVERIFY(msword.isValid());
    const QMimeType olestorage = db.mimeTypeForName("application/x-ole-storage");
    QVERIFY(olestorage.isValid());
    QVERIFY(msword.inherits(olestorage.name()));
    QVERIFY(msword.inherits("application/octet-stream"));

    const QMimeType directory = db.mimeTypeForName("inode/directory");
    QVERIFY(directory.isValid());
    QCOMPARE(directory.parentMimeTypes().count(), 0);
    QVERIFY(!directory.inherits("application/octet-stream"));

    // Check that text/x-patch knows that it inherits from text/plain (it says so explicitly)
    const QMimeType plain = db.mimeTypeForName("text/plain");
    const QMimeType derived = db.mimeTypeForName("text/x-patch");
    QVERIFY(derived.isValid());
    QCOMPARE(derived.parentMimeTypes().join(","), plain.name());
    QVERIFY(derived.inherits("text/plain"));
    QVERIFY(derived.inherits("application/octet-stream"));

    // Check that application/x-shellscript inherits from application/x-executable
    // (Otherwise KRun cannot start shellscripts...)
    // This is a test for multiple inheritance...
    const QMimeType shellscript = db.mimeTypeForName("application/x-shellscript");
    QVERIFY(shellscript.isValid());
    QVERIFY(shellscript.inherits("text/plain"));
    QVERIFY(shellscript.inherits("application/x-executable"));
    const QStringList shellParents = shellscript.parentMimeTypes();
    QVERIFY(shellParents.contains("text/plain"));
    QVERIFY(shellParents.contains("application/x-executable"));
    QCOMPARE(shellParents.count(), 2); // only the above two
    const QStringList allShellParents = shellscript.allParentMimeTypes();
    QVERIFY(allShellParents.contains("text/plain"));
    QVERIFY(allShellParents.contains("application/x-executable"));
    QVERIFY(allShellParents.contains("application/octet-stream"));
    // Must be least-specific last, i.e. breadth first.
    QCOMPARE(allShellParents.last(), QString("application/octet-stream"));

    // Check that text/x-mrml knows that it inherits from text/plain (implicitly)
    const QMimeType mrml = db.mimeTypeForName("text/x-mrml");
    QVERIFY(mrml.isValid());
    QVERIFY(mrml.inherits("text/plain"));
    QVERIFY(mrml.inherits("application/octet-stream"));
}

void tst_qmimetype::test_aliases()
{
    QMimeDatabase db;

    const QMimeType canonical = db.mimeTypeForName("application/xml");
    QVERIFY(canonical.isValid());

    QMimeType resolvedAlias = db.mimeTypeForName("text/xml");
    QVERIFY(resolvedAlias.isValid());
    QCOMPARE(resolvedAlias.name(), QString("application/xml"));

    QVERIFY(resolvedAlias.inherits("application/xml"));
    QVERIFY(canonical.inherits("text/xml"));

    // Test for kde bug 197346: does nspluginscan see that audio/mp3 already exists?
    bool mustWriteMimeType = !db.mimeTypeForName("audio/mp3").isValid();
    QVERIFY(!mustWriteMimeType);
}

// ------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    tst_qmimetype tc;
    return QTest::qExec(&tc, argc, argv);
}
