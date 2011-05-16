#ifndef QMIMEDATABASE_P_H
#define QMIMEDATABASE_P_H

#include "qmimedatabase.h"

#include <QtCore/QTextStream>
#include <QtCore/QMultiHash>

// XML tags in mime files
const char * const mimeInfoTagC = "mime-info";
const char * const mimeTypeTagC = "mime-type";
const char * const mimeTypeAttributeC = "type";
const char * const subClassTagC = "sub-class-of";
const char * const commentTagC = "comment";
const char * const globTagC = "glob";
const char * const aliasTagC = "alias";
const char * const patternAttributeC = "pattern";
const char * const weightAttributeC = "weight";
const char * const localeAttributeC = "xml:lang";

const char * const magicTagC = "magic";
const char * const priorityAttributeC = "priority";
const char * const matchTagC = "match";
const char * const matchValueAttributeC = "value";
const char * const matchTypeAttributeC = "type";
const char * const matchOffsetAttributeC = "offset";

// Types
const char * const textTypeC = "text/plain";
const char * const binaryTypeC = "application/octet-stream";

// MimeMapEntry: Entry of a type map, consisting of type and level.

enum { Dangling = 32767 };

struct MimeMapEntry
{
    explicit MimeMapEntry(const QMimeType &t = QMimeType(), int aLevel = Dangling) :
        type(t),
        level(aLevel)
    {
    }

    QMimeType type;
    int level; // hierachy level
};

class QMimeDatabasePrivate
{
    Q_DISABLE_COPY(QMimeDatabasePrivate)
    friend class QMimeDatabase;
public:
    QMimeDatabasePrivate();

    bool addMimeTypes(const QString &fileName, QString *errorMessage);
    bool addMimeTypes(QIODevice *device, QString *errorMessage);
    bool addMimeType(QMimeType mt);

    QMimeType findByType(const QString &type) const;
    QMimeType findByFile(const QFileInfo &f) const;
    QMimeType findByName(const QString &name) const;
    QMimeType findByData(const QByteArray &data) const;

    QStringList filterStrings() const;

    QStringList suffixes() const;
    bool setPreferredSuffix(const QString &typeOrAlias, const QString &suffix);

    QList<QMimeGlobPattern> globPatterns() const;
    void setGlobPatterns(const QString &typeOrAlias, const QList<QMimeGlobPattern> &globPatterns);

    QList<QSharedPointer<IMagicMatcher> > magicMatchers() const;
    void setMagicMatchers(const QString &typeOrAlias,
                          const QList<QSharedPointer<IMagicMatcher> > &matchers);

    QList<QMimeType> mimeTypes() const;

    void syncUserModifiedMimeTypes();
    static QList<QMimeType> readUserModifiedMimeTypes();
    static void writeUserModifiedMimeTypes(const QList<QMimeType> &mimeTypes);
    void clearUserModifiedMimeTypes();

    static QList<QMimeGlobPattern> toGlobPatterns(const QStringList &patterns,
                                                 int weight = QMimeGlobPattern::MaxWeight);
    static QStringList fromGlobPatterns(const QList<QMimeGlobPattern> &globPatterns);

    void debug(QTextStream &str) const;

//    QMimeType findByFileUnlocked(const QFileInfo &f) const;

private:
    typedef QHash<QString, MimeMapEntry> TypeMimeTypeMap;
    typedef QHash<QString, QString> AliasMap;
    typedef QMultiHash<QString, QString> ParentChildrenMap;

    static const QChar kSemiColon;
    static const QString kModifiedMimeTypesFile;
    static QString kModifiedMimeTypesPath;


    bool addMimeTypes(QIODevice *device, const QString &fileName, QString *errorMessage);
    inline QString resolveAlias(const QString &name) const;
    QMimeType findByFile(const QFileInfo &f, unsigned *priority) const;
    QMimeType findByData(const QByteArray &data, unsigned *priority) const;
    void determineLevels();
    void raiseLevelRecursion(MimeMapEntry &e, int level);

    TypeMimeTypeMap m_typeMimeTypeMap;
    AliasMap m_aliasMap;
    ParentChildrenMap m_parentChildrenMap;
    int m_maxLevel;
    QMutex m_mutex;
};

class BaseMimeTypeParser {
    Q_DISABLE_COPY(BaseMimeTypeParser)
public:
    BaseMimeTypeParser() {}
    virtual ~BaseMimeTypeParser() {}

    bool parse(QIODevice *dev, const QString &fileName, QString *errorMessage);

private:
    // Overwrite to process the sequence of parsed data
    virtual bool process(const QMimeType &t, QString *errorMessage) = 0;

    void addGlobPattern(const QString &pattern, const QString &weight, QMimeTypeData *d) const;

    enum ParseStage { ParseBeginning,
                      ParseMimeInfo,
                      ParseMimeType,
                      ParseComment,
                      ParseGlobPattern,
                      ParseSubClass,
                      ParseAlias,
                      ParseMagic,
                      ParseMagicMatchRule,
                      ParseOtherMimeTypeSubTag,
                      ParseError };

    static ParseStage nextStage(ParseStage currentStage, const QStringRef &startElement);
};

// Parser that builds MimeDB hierarchy by adding to MimeDatabasePrivate
class MimeTypeParser : public BaseMimeTypeParser {
public:
    explicit MimeTypeParser(QMimeDatabasePrivate &db) : m_db(db) {}
private:
    virtual bool process(const QMimeType &t, QString *) { m_db.addMimeType(t); return true; }

    QMimeDatabasePrivate &m_db;
};

#endif // QMIMEDATABASE_P_H
