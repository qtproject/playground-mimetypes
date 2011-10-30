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

import Qt 4.7
import QtTest 1.0
import QtMimeTypes 1.0

import "tst_qdeclarativemimetypedata.js" as Data

TestCase {
    name: "tst_QDeclarativeMimeType"

    MimeType {
        id: instantiatedPngMimeType
        name: Data.mimeTypeName()
        comment: Data.mimeTypeComment()
        genericIconName: Data.mimeTypeGenericIconName()
        iconName: Data.mimeTypeIconName()
        globPatterns: Data.mimeTypeGlobPatterns()
        suffixes: Data.mimeTypeSuffixes()
    }

    MimeType {
        id: otherPngMimeType
    }

    MimeType {
        id: defaultMimeType
    }

    function test_isValid() {
        compare(instantiatedPngMimeType.isValid, true)

        otherPngMimeType.assign(instantiatedPngMimeType)

        compare(otherPngMimeType.isValid, true)
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)
        compare(otherPngMimeType.equals(instantiatedPngMimeType), true)

        compare(defaultMimeType.isValid, false)
    }

    function test_Elements() {
        otherPngMimeType.assign(instantiatedPngMimeType)
        compare(otherPngMimeType.equals(instantiatedPngMimeType), true)
        compare(otherPngMimeType.equals(defaultMimeType), false)
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)
        compare(defaultMimeType.equals(otherPngMimeType), false)

        otherPngMimeType.assign(defaultMimeType)
        compare(otherPngMimeType.equals(instantiatedPngMimeType), false)
        compare(otherPngMimeType.equals(defaultMimeType), true)
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)
        compare(defaultMimeType.equals(otherPngMimeType), true)
    }

    function test_JavaScriptObjects() {
        otherPngMimeType.assign(instantiatedPngMimeType)   // to uncover problems in assignProperties()
        var javaScriptObject = new Object

        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), false)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)

        javaScriptObject.name = Data.mimeTypeName()
        javaScriptObject.comment = Data.mimeTypeComment()
        javaScriptObject.genericIconName = Data.mimeTypeGenericIconName()
        javaScriptObject.iconName = Data.mimeTypeIconName()
        javaScriptObject.globPatterns = Data.mimeTypeGlobPatterns()
        javaScriptObject.suffixes = Data.mimeTypeSuffixes()
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), true)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)
    }

    function test_name() {
        // Verify that the Name is part of the equality test:
        //compare(instantiatedPngMimeType.name, Data.mimeTypeName())
        compare(Data.equalMimeTypeName(instantiatedPngMimeType.name, Data.mimeTypeName()), true)

        otherPngMimeType.assign(instantiatedPngMimeType)

        otherPngMimeType.name = defaultMimeType.name   // simulate an error
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)
        otherPngMimeType.name = instantiatedPngMimeType.name
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)

        var javaScriptObject = instantiatedPngMimeType.properties()

        javaScriptObject.name = defaultMimeType.name   // simulate an error
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), false)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)

        javaScriptObject.name = instantiatedPngMimeType.name
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), true)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)
    }

    function test_comment() {
        // Verify that the Comment is part of the equality test:
        //compare(instantiatedPngMimeType.comment, Data.mimeTypeComment())
        compare(Data.equalMimeTypeComment(instantiatedPngMimeType.comment, Data.mimeTypeComment()), true)

        otherPngMimeType.assign(instantiatedPngMimeType)

        otherPngMimeType.comment = defaultMimeType.comment   // simulate an error
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)
        otherPngMimeType.comment = instantiatedPngMimeType.comment
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)

        var javaScriptObject = instantiatedPngMimeType.properties()

        javaScriptObject.comment = defaultMimeType.comment   // simulate an error
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), false)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)

        javaScriptObject.comment = instantiatedPngMimeType.comment
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), true)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)
    }

    function test_genericIconName() {
        // Verify that the GenericIconName is part of the equality test:
        //compare(instantiatedPngMimeType.genericIconName, Data.mimeTypeGenericIconName())
        compare(Data.equalMimeTypeGenericIconName(instantiatedPngMimeType.genericIconName, Data.mimeTypeGenericIconName()), true)

        otherPngMimeType.assign(instantiatedPngMimeType)

        otherPngMimeType.genericIconName = defaultMimeType.genericIconName   // simulate an error
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)
        otherPngMimeType.genericIconName = instantiatedPngMimeType.genericIconName
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)

        var javaScriptObject = instantiatedPngMimeType.properties()

        javaScriptObject.genericIconName = defaultMimeType.genericIconName   // simulate an error
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), false)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)

        javaScriptObject.genericIconName = instantiatedPngMimeType.genericIconName
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), true)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)
    }

    function test_iconName() {
        // Verify that the IconName is part of the equality test:
        //compare(instantiatedPngMimeType.iconName, Data.mimeTypeIconName())
        compare(Data.equalMimeTypeIconName(instantiatedPngMimeType.iconName, Data.mimeTypeIconName()), true)

        otherPngMimeType.assign(instantiatedPngMimeType)

        otherPngMimeType.iconName = defaultMimeType.iconName   // simulate an error
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)
        otherPngMimeType.iconName = instantiatedPngMimeType.iconName
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)

        var javaScriptObject = instantiatedPngMimeType.properties()

        javaScriptObject.iconName = defaultMimeType.iconName   // simulate an error
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), false)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)

        javaScriptObject.iconName = instantiatedPngMimeType.iconName
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), true)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)
    }

    function test_globPatterns() {
        // Verify that the GlobPatterns is part of the equality test:
        //compare(instantiatedPngMimeType.globPatterns, Data.mimeTypeGlobPatterns())
        compare(Data.equalMimeTypeGlobPatterns(instantiatedPngMimeType.globPatterns, Data.mimeTypeGlobPatterns()), true)

        otherPngMimeType.assign(instantiatedPngMimeType)

        otherPngMimeType.globPatterns = defaultMimeType.globPatterns   // simulate an error
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)
        otherPngMimeType.globPatterns = instantiatedPngMimeType.globPatterns
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)

        var javaScriptObject = instantiatedPngMimeType.properties()

        javaScriptObject.globPatterns = defaultMimeType.globPatterns   // simulate an error
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), false)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)

        javaScriptObject.globPatterns = instantiatedPngMimeType.globPatterns
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), true)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)
    }

    function test_suffixes() {
        // Verify that the Suffixes is part of the equality test:
        //compare(instantiatedPngMimeType.suffixes, Data.mimeTypeSuffixes())
        compare(Data.equalMimeTypeSuffixes(instantiatedPngMimeType.suffixes, Data.mimeTypeSuffixes()), true)

        otherPngMimeType.assign(instantiatedPngMimeType)

        otherPngMimeType.suffixes = defaultMimeType.suffixes   // simulate an error
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)
        otherPngMimeType.suffixes = instantiatedPngMimeType.suffixes
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)

        var javaScriptObject = instantiatedPngMimeType.properties()

        javaScriptObject.suffixes = defaultMimeType.suffixes   // simulate an error
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), false)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)

        javaScriptObject.suffixes = instantiatedPngMimeType.suffixes
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), true)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)
    }
}
