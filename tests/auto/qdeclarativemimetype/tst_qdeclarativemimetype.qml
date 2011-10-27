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

TestCase {
    name: "tst_QDeclarativeMimetype"

    function pngMimeTypeName() {
        return "image/png"
    }

    function pngMimeTypeComment() {
        return "PNG File";
    }

    function pngMimeTypeGenericIconName() {
        return "/usr/share/icons/oxygen/64x64/mimetypes/image-x-generic.png"
    }

    function pngMimeTypeIconName() {
        return "/usr/share/icons/oxygen/64x64/mimetypes/image-x-generic.png"
    }

    function firstPngMimeTypeSuffixes() {
        return ".png"
    }

    MimeType {
        id: instantiatedPngMimeType
        name: pngMimeTypeName()
        comment: pngMimeTypeComment()
        genericIconName: pngMimeTypeGenericIconName()
        iconName: pngMimeTypeIconName()
        suffixes: [ firstPngMimeTypeSuffixes() ]
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

    function test_name() {
        // Verify that the Name is part of the equality test:
        compare(instantiatedPngMimeType.name, pngMimeTypeName())

        otherPngMimeType.assign(instantiatedPngMimeType)

        otherPngMimeType.name = ""   // simulate an error
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)
        otherPngMimeType.name = instantiatedPngMimeType.name
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)

        var javaScriptObject = instantiatedPngMimeType.properties()
        
        javaScriptObject.name = ""   // simulate an error
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
        compare(instantiatedPngMimeType.comment, pngMimeTypeComment())

        otherPngMimeType.assign(instantiatedPngMimeType)

        otherPngMimeType.comment = ""   // simulate an error
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)
        otherPngMimeType.comment = instantiatedPngMimeType.comment
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)

        var javaScriptObject = instantiatedPngMimeType.properties()
        
        javaScriptObject.comment = ""   // simulate an error
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
        compare(instantiatedPngMimeType.genericIconName, pngMimeTypeGenericIconName())

        otherPngMimeType.assign(instantiatedPngMimeType)

        otherPngMimeType.genericIconName = ""   // simulate an error
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)
        otherPngMimeType.genericIconName = instantiatedPngMimeType.genericIconName
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)

        var javaScriptObject = instantiatedPngMimeType.properties()
        
        javaScriptObject.genericIconName = ""   // simulate an error
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
        compare(instantiatedPngMimeType.iconName, pngMimeTypeIconName())

        otherPngMimeType.assign(instantiatedPngMimeType)

        otherPngMimeType.iconName = ""
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)
        otherPngMimeType.iconName = instantiatedPngMimeType.iconName
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)

        var javaScriptObject = instantiatedPngMimeType.properties()

        javaScriptObject.iconName = ""   // simulate an error
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), false)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)

        javaScriptObject.iconName = instantiatedPngMimeType.iconName
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), true)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)
    }

    function test_suffixes() {
        // Verify that the Suffixes is part of the equality test:
        compare(instantiatedPngMimeType.suffixes.length, 1)
        compare(instantiatedPngMimeType.suffixes[0], ".png")

        otherPngMimeType.assign(instantiatedPngMimeType)

        otherPngMimeType.suffixes = []
        compare(otherPngMimeType.suffixes.length, 0)
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)
        otherPngMimeType.suffixes = instantiatedPngMimeType.suffixes
        compare(otherPngMimeType.suffixes.length, 1)
        compare(otherPngMimeType.suffixes[0], ".png")
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)

        var javaScriptObject = instantiatedPngMimeType.properties()

        javaScriptObject.suffixes = []   // simulate an error
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), false)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), false)

        javaScriptObject.suffixes = instantiatedPngMimeType.suffixes
        compare(instantiatedPngMimeType.equalsProperties(javaScriptObject), true)
        otherPngMimeType.assignProperties(javaScriptObject);
        compare(instantiatedPngMimeType.equals(otherPngMimeType), true)
    }
}
