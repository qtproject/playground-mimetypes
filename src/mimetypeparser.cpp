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

#include "qmimedatabase_p.h"

#include "qmimetype_p.h"
#include "magicmatcher_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

/*!
    \class BaseMimeTypeParser
    \brief Generic parser for a sequence of <mime-type>.

    Calls abstract handler function process for MimeType it finds.

    \sa MimeDatabase, IMagicMatcher, MagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa FileMatchContext, BinaryMatcher, HeuristicTextMagicMatcher
    \sa MimeTypeParser
*/

//using namespace Internal;

//namespace Internal {

void BaseMimeTypeParser::addGlobPattern(const QString &pattern, const QString &weight, QMimeTypeData *d) const
{
    if (pattern.isEmpty())
        return;
    // Collect patterns as a QRegExp list and filter out the plain
    // suffix ones for our suffix list. Use first one as preferred
    const QRegExp wildCard(pattern, Qt::CaseSensitive, QRegExp::Wildcard);
    if (!wildCard.isValid()) {
        qWarning("%s: Invalid wildcard '%s'.",
                 Q_FUNC_INFO, pattern.toUtf8().constData());
        return;
    }

    if (weight.isEmpty())
        d->globPatterns.push_back(MimeGlobPattern(wildCard));
    else
        d->globPatterns.push_back(MimeGlobPattern(wildCard, weight.toInt()));

    d->assignSuffix(pattern);
}

BaseMimeTypeParser::ParseStage BaseMimeTypeParser::nextStage(ParseStage currentStage,
                                                             const QStringRef &startElement)
{
    switch (currentStage) {
    case ParseBeginning:
        if (startElement == QLatin1String(mimeInfoTagC))
            return ParseMimeInfo;
        if (startElement == QLatin1String(mimeTypeTagC))
            return ParseMimeType;
        return ParseError;
    case ParseMimeInfo:
        return startElement == QLatin1String(mimeTypeTagC) ? ParseMimeType : ParseError;
    case ParseMimeType:
    case ParseComment:
    case ParseGlobPattern:
    case ParseSubClass:
    case ParseAlias:
    case ParseOtherMimeTypeSubTag:
    case ParseMagicMatchRule:
        if (startElement == QLatin1String(mimeTypeTagC)) // Sequence of <mime-type>
            return ParseMimeType;
        if (startElement == QLatin1String(commentTagC ))
            return ParseComment;
        if (startElement == QLatin1String(globTagC))
            return ParseGlobPattern;
        if (startElement == QLatin1String(subClassTagC))
            return ParseSubClass;
        if (startElement == QLatin1String(aliasTagC))
            return ParseAlias;
        if (startElement == QLatin1String(magicTagC))
            return ParseMagic;
        if (startElement == QLatin1String(matchTagC))
            return ParseMagicMatchRule;
        return ParseOtherMimeTypeSubTag;
    case ParseMagic:
        if (startElement == QLatin1String(matchTagC))
            return ParseMagicMatchRule;
        break;
    case ParseError:
        break;
    }
    return ParseError;
}

// Parse int number from an (attribute) string)
static bool parseNumber(const QString &n, int *target, QString *errorMessage)
{
    bool ok;
    *target = n.toInt(&ok);
    if (!ok) {
        *errorMessage = QString::fromLatin1("Not a number '%1'.").arg(n);
        return false;
    }
    return true;
}

// Evaluate a magic match rule like
//  <match value="must be converted with BinHex" type="string" offset="11"/>
//  <match value="0x9501" type="big16" offset="0:64"/>
static bool addMagicMatchRule(const QXmlStreamAttributes &atts,
                              const MagicRuleMatcherPtr  &ruleMatcher,
                              QString *errorMessage)
{
    const QString type = atts.value(QLatin1String(matchTypeAttributeC)).toString();
    QMimeMagicRule::Type magicType = QMimeMagicRule::stringToType(type);
    if (magicType == QMimeMagicRule::Unknown) {
        qWarning("%s: match type %s is not supported.", Q_FUNC_INFO, type.toUtf8().constData());
        return true;
    }
    const QString value = atts.value(QLatin1String(matchValueAttributeC)).toString();
    if (value.isEmpty()) {
        *errorMessage = QString::fromLatin1("Empty match value detected.");
        return false;
    }
    // Parse for offset as "1" or "1:10"
    int startPos, endPos;
    const QString offsetS = atts.value(QLatin1String(matchOffsetAttributeC)).toString();
    const int colonIndex = offsetS.indexOf(QLatin1Char(':'));
    const QString startPosS = colonIndex == -1 ? offsetS : offsetS.mid(0, colonIndex);
    const QString endPosS   = colonIndex == -1 ? offsetS : offsetS.mid(colonIndex + 1);
    if (!parseNumber(startPosS, &startPos, errorMessage) || !parseNumber(endPosS, &endPos, errorMessage))
        return false;
    if (debugMimeDB)
        qDebug() << Q_FUNC_INFO << value << startPos << endPos;

    ruleMatcher->add(QMimeMagicRule(magicType, value, startPos, endPos));
    return true;
}

bool BaseMimeTypeParser::parse(QIODevice *dev, const QString &fileName, QString *errorMessage)
{
    QMimeTypeData data;
    MagicRuleMatcherPtr ruleMatcher;
    QXmlStreamReader reader(dev);
    ParseStage ps = ParseBeginning;
    QXmlStreamAttributes atts;
    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::StartElement:
            ps = nextStage(ps, reader.name());
            atts = reader.attributes();
            switch (ps) {
            case ParseMimeType: { // start parsing a type
                const QString type = atts.value(QLatin1String(mimeTypeAttributeC)).toString();
                if (type.isEmpty()) {
                    reader.raiseError(QString::fromLatin1("Missing 'type'-attribute"));
                } else {
                    data.type = type;
                }
            }
                break;
            case ParseGlobPattern:
                addGlobPattern(atts.value(QLatin1String(patternAttributeC)).toString(),
                               atts.value(QLatin1String(weightAttributeC)).toString(), &data);
                break;
            case ParseSubClass: {
                const QString inheritsFrom = atts.value(QLatin1String(mimeTypeAttributeC)).toString();
                if (!inheritsFrom.isEmpty())
                    data.subClassesOf.push_back(inheritsFrom);
            }
                break;
            case ParseComment: {
                // comments have locale attributes. We want the default, English one
                QString locale = atts.value(QLatin1String(localeAttributeC)).toString();
                const QString comment = QCoreApplication::translate("MimeType",
                                                                    reader.readElementText().toAscii());
                if (locale.isEmpty()) {
                    data.comment = comment;
                } else {
                    data.localeComments.insert(locale, comment);
                }
            }
                break;
            case ParseAlias: {
                const QString alias = atts.value(QLatin1String(mimeTypeAttributeC)).toString();
                if (!alias.isEmpty())
                    data.aliases.push_back(alias);
            }
                break;
            case ParseMagic: {
                int priority = 0;
                const QString priorityS = atts.value(QLatin1String(priorityAttributeC)).toString();
                if (!priorityS.isEmpty()) {
                    if (!parseNumber(priorityS, &priority, errorMessage))
                        return false;

                }
                ruleMatcher = MagicRuleMatcherPtr(new MagicRuleMatcher);
                ruleMatcher->setPriority(priority);
            }
                break;
            case ParseMagicMatchRule:
                if (ruleMatcher.isNull()) {
                    qWarning() << "BaseMimeTypeParser::parse : ruleMatcher unexpectedly null";
                    return false;
                }
                if (!addMagicMatchRule(atts, ruleMatcher, errorMessage))
                    return false;
                break;
            case ParseError:
                reader.raiseError(QString::fromLatin1("Unexpected element <%1>").
                                  arg(reader.name().toString()));
                break;
            default:
                break;
            } // switch nextStage
            break;
        // continue switch QXmlStreamReader::Token...
        case QXmlStreamReader::EndElement: // Finished element
            if (reader.name() == QLatin1String(mimeTypeTagC)) {
                if (!process(QMimeType(data), errorMessage))
                    return false;
                data.clear();
            } else {
                // Finished a match sequence
                if (reader.name() == QLatin1String(magicTagC)) {
                    if (ruleMatcher.isNull()) {
                        qWarning() << "BaseMimeTypeParser::parse : ruleMatcher unexpectedly null";
                        return false;
                    }
                    data.magicMatchers.push_back(ruleMatcher);
                    ruleMatcher = MagicRuleMatcherPtr();
                }
            }
            break;

        default:
            break;
        } // switch reader.readNext()
    }

    if (reader.hasError()) {
        *errorMessage = QString::fromLatin1("An error has been encountered at line %1 of %2: %3:").arg(reader.lineNumber()).arg(fileName, reader.errorString());
        return false;
    }
    return true;
}

//} // namespace Internal
