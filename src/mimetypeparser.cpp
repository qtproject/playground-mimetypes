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
#include <QtCore/QDir>
#include <QtCore/QPair>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

QT_BEGIN_NAMESPACE

// XML tags in mime files
const char *const mimeInfoTagC = "mime-info";
const char *const mimeTypeTagC = "mime-type";
const char *const mimeTypeAttributeC = "type";
const char *const subClassTagC = "sub-class-of";
const char *const commentTagC = "comment";
const char *const globTagC = "glob";
const char *const aliasTagC = "alias";
const char *const patternAttributeC = "pattern";
const char *const weightAttributeC = "weight";
const char *const caseSensitiveAttributeC = "case-sensitive";
const char *const localeAttributeC = "xml:lang";

const char *const magicTagC = "magic";
const char *const priorityAttributeC = "priority";

const char *const matchTagC = "match";
const char *const matchValueAttributeC = "value";
const char *const matchTypeAttributeC = "type";
const char *const matchOffsetAttributeC = "offset";
const char *const matchMaskAttributeC = "mask";

/*!
    \class MimeTypeParser
    \brief Mime type parser

    Populates MimeDataBase

    \sa MimeDatabase, IMagicMatcher, MagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa FileMatchContext, BinaryMatcher, HeuristicTextMagicMatcher
    \sa MimeTypeParser
*/


/*!
    \class BaseMimeTypeParser
    \brief Generic parser for a sequence of <mime-type>.

    Calls abstract handler function process for MimeType it finds.

    \sa MimeDatabase, IMagicMatcher, MagicRuleMatcher, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa FileMatchContext, BinaryMatcher, HeuristicTextMagicMatcher
    \sa MimeTypeParser
*/

static inline void addGlobPattern(const QRegExp &wildCard, unsigned weight, QMimeTypeData *d)
{
    // Collect patterns as a QRegExp list and filter out the plain
    // suffix ones for our suffix list. Use first one as preferred
    if (!wildCard.isValid()) {
        qWarning("%s: Invalid wildcard '%s'.", Q_FUNC_INFO, wildCard.pattern().toLocal8Bit().constData());
        return;
    }

    if (weight == 0)
        weight = QMimeGlobPattern::DefaultWeight;

    d->globPatterns.append(QMimeGlobPattern(wildCard, weight));

    d->assignSuffix(wildCard.pattern());
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
                              QString *errorMessage, QMimeMagicRule *&rule)
{
    const QString type = atts.value(QLatin1String(matchTypeAttributeC)).toString();
    QMimeMagicRule::Type magicType = QMimeMagicRule::type(type.toLatin1());
    if (magicType == QMimeMagicRule::Invalid) {
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
    const QString mask = atts.value(QLatin1String(matchMaskAttributeC)).toString();

    if (debugMimeDB)
        qDebug() << Q_FUNC_INFO << value << startPos << endPos << mask;

//    ruleMatcher->addRule(QMimeMagicRule(magicType, value.toUtf8(), startPos, endPos, mask.toLatin1()));
    rule = new QMimeMagicRule(magicType, value.toUtf8(), startPos, endPos, mask.toLatin1());

    return true;
}

bool BaseMimeTypeParser::parse(QIODevice *dev, const QString &fileName, QString *errorMessage)
{
    QMimeTypeData data;
    QMimeMagicRuleMatcher *ruleMatcher = 0;
    int priority = 50;
    QList<QMimeMagicRule> rules;
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
            case ParseGlobPattern: {
                const QString pattern = atts.value(QLatin1String(patternAttributeC)).toString();
                const unsigned weight = atts.value(QLatin1String(weightAttributeC)).toString().toInt();
                const bool caseSensitive = atts.value(QLatin1String(caseSensitiveAttributeC)).toString() == QLatin1String("true");

                const QRegExp wildCard(pattern, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::WildcardUnix);
                addGlobPattern(wildCard, weight, &data);
            }
                break;
            case ParseSubClass: {
                const QString inheritsFrom = atts.value(QLatin1String(mimeTypeAttributeC)).toString();
                if (!inheritsFrom.isEmpty())
                    data.subClassOf.push_back(inheritsFrom);
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
                priority = 50;
                const QString priorityS = atts.value(QLatin1String(priorityAttributeC)).toString();
                if (!priorityS.isEmpty()) {
                    if (!parseNumber(priorityS, &priority, errorMessage))
                        return false;

                }
//                ruleMatcher = MagicRuleMatcherPtr(new MagicRuleMatcher);
//                ruleMatcher->setPriority(priority);
            }
                break;
            case ParseMagicMatchRule: {
//                if (ruleMatcher.isNull()) {
//                    qWarning() << "BaseMimeTypeParser::parse : ruleMatcher unexpectedly null";
//                    return false;
//                }
                if (!ruleMatcher) {
                    ruleMatcher = new QMimeMagicRuleMatcher;
                    ruleMatcher->setPriority(priority);
                }

                QMimeMagicRule *rule = 0;
                if (!addMagicMatchRule(atts, /*ruleMatcher, */errorMessage, rule))
                    return false;
                rules.append(*rule);
                delete rule;
                break;
            }
            case ParseError:
                reader.raiseError(QString::fromLatin1("Unexpected element <%1>").
                                  arg(reader.name().toString()));
                break;
            default:
                break;
            }
            break;
        // continue switch QXmlStreamReader::Token...
        case QXmlStreamReader::EndElement: // Finished element
            if (reader.name() == QLatin1String(mimeTypeTagC)) {
                if (!process(QMimeType(data), errorMessage))
                    return false;
                data.clear();
            } else {
                // Finished a match sequence
//                if (reader.name() == QLatin1String(magicTagC)) {
//                    if (ruleMatcher.isNull()) {
//                        qWarning() << "BaseMimeTypeParser::parse : ruleMatcher unexpectedly null";
//                        return false;
//                    }
//                    data.magicMatchers.push_back(ruleMatcher);
//                    ruleMatcher = MagicRuleMatcherPtr();
//                }

                // Finished a match sequence
                if (reader.name() == QLatin1String(matchTagC)) {
                    if (ruleMatcher) {
                        ruleMatcher->addRules(rules);
                        data.magicMatchers.push_back(*ruleMatcher);
                        ruleMatcher = 0;
                    }
                    rules.takeLast();
                }
            }
            break;

        default:
            break;
        }
    }

    if (reader.hasError()) {
        if (errorMessage)
            *errorMessage = QString::fromLatin1("An error has been encountered at line %1 of %2: %3:").arg(reader.lineNumber()).arg(fileName, reader.errorString());
        return false;
    }

    return true;
}

static inline QString toOffset_helper(const QPair<int, int> &startEnd)
{
    return QString::number(startEnd.first) + QLatin1Char(':') + QString::number(startEnd.second);
}

static inline QPair<int, int> fromOffset_helper(const QString &offset)
{
    const QStringList startEnd = offset.split(QLatin1Char(':'));
    Q_ASSERT(startEnd.size() == 2);
    return qMakePair(startEnd.at(0).toInt(), startEnd.at(1).toInt());
}

QList<QMimeType> QMimeDatabasePrivate::readUserModifiedMimeTypes()
{
    QList<QMimeType> mimeTypes;
    QFile file(kModifiedMimeTypesPath + kModifiedMimeTypesFile);
    if (file.open(QFile::ReadOnly)) {
        QMimeType mimeType;
        QHash<int, QList<QMimeMagicRule> > rules;
        QXmlStreamReader reader(&file);
        QXmlStreamAttributes atts;
        while (!reader.atEnd()) {
            switch (reader.readNext()) {
            case QXmlStreamReader::StartElement:
                atts = reader.attributes();
                if (reader.name() == mimeTypeTagC) {
                    mimeType.setType(atts.value(QLatin1String(mimeTypeAttributeC)).toString());
                    const QString &patterns = atts.value(QLatin1String(patternAttributeC)).toString();
                    mimeType.setGlobPatterns(toGlobPatterns(patterns.split(QLatin1Char(';'))));
                } else if (reader.name() == matchTagC) {
                    const QString value = atts.value(QLatin1String(matchValueAttributeC)).toString();
                    const QString type = atts.value(QLatin1String(matchTypeAttributeC)).toString();
                    const QString offset = atts.value(QLatin1String(matchOffsetAttributeC)).toString();
                    const QString mask = atts.value(QLatin1String(matchMaskAttributeC)).toString();
                    QPair<int, int> range = fromOffset_helper(offset);
                    const int priority = atts.value(QLatin1String(priorityAttributeC)).toString().toInt();

                    QMimeMagicRule::Type magicType = QMimeMagicRule::type(type.toLatin1());
                    if (magicType != QMimeMagicRule::Invalid)
                        rules[priority].append(QMimeMagicRule(magicType, value.toUtf8(), range.first, range.second, mask.toLatin1()));
                }
                break;
            case QXmlStreamReader::EndElement:
                if (reader.name() == mimeTypeTagC) {
                    QList<QMimeMagicRuleMatcher> matchers;
                    QHash<int, QList<QMimeMagicRule> >::const_iterator it = rules.constBegin();
                    for ( ; it != rules.constEnd(); ++it) {
                        QMimeMagicRuleMatcher magicRuleMatcher;
                        magicRuleMatcher.setPriority(it.key());
                        magicRuleMatcher.addRules(it.value());
                        matchers.append(magicRuleMatcher);
                    }
                    mimeType.setMagicMatchers(matchers);
                    mimeTypes.append(mimeType);
                    mimeType.clear();
                    rules.clear();
                }
                break;
            default:
                break;
            }
        }
        if (reader.hasError())
            qWarning() << kModifiedMimeTypesFile << reader.errorString() << reader.lineNumber()
                       << reader.columnNumber();
        file.close();
    }
    return mimeTypes;
}

void QMimeDatabasePrivate::writeUserModifiedMimeTypes(const QList<QMimeType> &mimeTypes)
{
    // Keep mime types modified which are already on file, unless they are part of the current set.
    QSet<QString> currentMimeTypes;
    foreach (const QMimeType &mimeType, mimeTypes)
        currentMimeTypes.insert(mimeType.type());
    const QList<QMimeType> &inFileMimeTypes = QMimeDatabasePrivate::readUserModifiedMimeTypes();
    QList<QMimeType> allModifiedMimeTypes = mimeTypes;
    foreach (const QMimeType &mimeType, inFileMimeTypes)
        if (!currentMimeTypes.contains(mimeType.type()))
            allModifiedMimeTypes.append(mimeType);

    if (QFile::exists(kModifiedMimeTypesPath) || QDir().mkpath(kModifiedMimeTypesPath)) {
        QFile file(kModifiedMimeTypesPath + kModifiedMimeTypesFile);
        if (file.open(QFile::WriteOnly | QFile::Truncate)) {
            // Notice this file only represents user modifications. It is writen in a
            // convienient way for synchronization, which is similar to but not exactly the
            // same format we use for the embedded mime type files.
            QXmlStreamWriter writer(&file);
            writer.setAutoFormatting(true);
            writer.writeStartDocument();
            writer.writeStartElement(QLatin1String(mimeInfoTagC));
            foreach (const QMimeType &mimeType, allModifiedMimeTypes) {
                writer.writeStartElement(QLatin1String(mimeTypeTagC));
                writer.writeAttribute(QLatin1String(mimeTypeAttributeC), mimeType.type());
                writer.writeAttribute(QLatin1String(patternAttributeC),
                                      fromGlobPatterns(mimeType.globPatterns()).join(QLatin1String(";")));
                foreach (const QMimeMagicRuleMatcher &matcher, mimeType.magicMatchers()) {
                    // Only care about rule-based matchers.
                    const QMimeMagicRuleMatcher *ruleMatcher = &matcher;
//                    if (QMimeMagicRuleMatcher *ruleMatcher =
//                        dynamic_cast<QMimeMagicRuleMatcher *>(matcher.data())) {
                        const QList<QMimeMagicRule> &rules = ruleMatcher->magicRules();
                        foreach (const QMimeMagicRule &rule, rules) {
                            writer.writeStartElement(QLatin1String(matchTagC));
                            writer.writeAttribute(QLatin1String(matchValueAttributeC), QString::fromUtf8(rule.value().constData()));
                            writer.writeAttribute(QLatin1String(matchTypeAttributeC), QString::fromLatin1(QMimeMagicRule::typeName(rule.type()).constData()));
                            writer.writeAttribute(QLatin1String(matchOffsetAttributeC),
                                                  toOffset_helper(qMakePair(rule.startPos(), rule.endPos())));
                            writer.writeAttribute(QLatin1String(matchMaskAttributeC), QString::fromLatin1(rule.mask().constData()));
                            writer.writeAttribute(QLatin1String(priorityAttributeC),
                                                  QString::number(ruleMatcher->priority()));
                            writer.writeEndElement();
                        }
//                    }
                }
                writer.writeEndElement();
            }
            writer.writeEndElement();
            writer.writeEndDocument();
            file.close();
        }
    }
}

QT_END_NAMESPACE
