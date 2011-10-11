#include "qmimeprovider_p.h"
#include "mimetypeparser_p.h"
#include <qstandardpaths.h>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <qendian.h>

QMimeProviderBase::QMimeProviderBase(QMimeDatabasePrivate *db)
    : m_db(db)
{
}

QMimeBinaryProvider::QMimeBinaryProvider(QMimeDatabasePrivate *db)
    : QMimeProviderBase(db)
{
}

#if defined(Q_OS_UNIX) && !defined(Q_OS_INTEGRITY)
#define QT_USE_MMAP
#endif

struct QMimeBinaryProvider::CacheFile {
    CacheFile(QFile *file);
    ~CacheFile();

    bool isValid() const { return m_valid; }
    inline quint16 getUint16(int offset) const {
        return qFromBigEndian(*reinterpret_cast<quint16 *>(data + offset));
    }
    inline quint32 getUint32(int offset) const {
        return qFromBigEndian(*reinterpret_cast<quint32 *>(data + offset));
    }
    inline const char* getCharStar(int offset) const {
        return reinterpret_cast<const char *>(data + offset);
    }

    QFile *file;
    uchar *data;
    bool m_valid;
};

QMimeBinaryProvider::CacheFile::CacheFile(QFile *f)
    : file(f), m_valid(false)
{
    data = file->map(0, file->size());
    if (data) {
        const int major = getUint16(0);
        const int minor = getUint16(2);
        m_valid = (major == 1 && minor >= 1 && minor <= 2);
    }
}

QMimeBinaryProvider::CacheFile::~CacheFile()
{
    delete file;
}

QMimeBinaryProvider::~QMimeBinaryProvider()
{
    qDeleteAll(m_cacheFiles);
}

// Position of the "list offsets" values, at the beginning of the mime.cache file
enum { PosAliasListOffset = 4,
       PosParentListOffset = 8,
       PosLiteralListOffset = 12,
       PosReverseSuffixTreeOffset = 16,
       PosGlobListOffset = 20,
       PosMagicListOffset = 24,
       // PosNamespaceListOffset = 28,
       PosIconsListOffset = 32,
       PosGenericIconsListOffset = 36
     };

bool QMimeBinaryProvider::isValid()
{
#if defined(QT_USE_MMAP)
    return false; // HACK FOR NOW

    const QStringList cacheFilenames = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QLatin1String("mime/mime.cache"));
    qDeleteAll(m_cacheFiles);
    m_cacheFiles.clear();

    // Verify version
    foreach (const QString& cacheFile, cacheFilenames) {
        QFile *file = new QFile(cacheFile);
        if (file->open(QIODevice::ReadOnly)) {
            CacheFile *cacheFile = new CacheFile(file);
            if (cacheFile->isValid())
                m_cacheFiles.append(cacheFile);
            else
                delete cacheFile;
        } else
            delete file;
    }

    if (m_cacheFiles.count() > 1)
        return true;
    if (m_cacheFiles.isEmpty())
        return false;

    // We found exactly one file; is it the user-modified mimes, or a system file?
    const QString foundFile = m_cacheFiles.first()->file->fileName();
    const QString localCacheFile = QStandardPaths::storageLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/mime/mime.cache");

    return foundFile != localCacheFile;
#else
    return false;
#endif
}

void QMimeBinaryProvider::ensureTypesLoaded()
{
}

QStringList QMimeBinaryProvider::findByName(const QString &fileName, QString *foundSuffix)
{
    GlobMatchResult results;
    results.weight = 0;
    results.matchingPatternLength = 0;
    foreach (CacheFile *cacheFile, m_cacheFiles) {
        matchGlobList(results, cacheFile, cacheFile->getUint32(PosLiteralListOffset), fileName);
        matchGlobList(results, cacheFile, cacheFile->getUint32(PosGlobListOffset), fileName);
        const int reverseSuffixTreeOffset = cacheFile->getUint32(PosReverseSuffixTreeOffset);
        const int numRoots = cacheFile->getUint32(reverseSuffixTreeOffset);
        const int firstRootOffset = cacheFile->getUint32(reverseSuffixTreeOffset + 4);
        matchSuffixTree(results, cacheFile, numRoots, firstRootOffset, fileName);
    }
    *foundSuffix = results.foundSuffix;
    return results.matchingMimeTypes;
}

void QMimeBinaryProvider::matchGlobList(GlobMatchResult& results, CacheFile *cacheFile, int off, const QString &fileName)
{
    const QString lowerCaseFileName = fileName;
    const int numGlobs = cacheFile->getUint32(off);
    //qDebug() << "Loading" << numGlobs << "globs from" << cacheFile->file->fileName() << "at offset" << cacheFile->globListOffset;
    for (int i = 0; i < numGlobs; ++i) {
        const int globOffset = cacheFile->getUint32(off + 4 + 12 * i);
        const int mimeTypeOffset = cacheFile->getUint32(off + 4 + 12 * i + 4);
        const int flagsAndWeight = cacheFile->getUint32(off + 4 + 12 * i + 8);
        const int weight = flagsAndWeight & 0xff;
        const bool caseSensitive = flagsAndWeight & 0x100;
        const Qt::CaseSensitivity qtCaseSensitive = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
        const QString pattern = QLatin1String(cacheFile->getCharStar(globOffset));

        const char* mimeType = cacheFile->getCharStar(mimeTypeOffset);
        qDebug() << pattern << mimeType << weight << caseSensitive;
        QMimeGlobPattern glob(pattern, QString() /*unused*/, weight, qtCaseSensitive);

        // TODO: this could be done faster for literals where a simple == would do.
        if (glob.matchFileName(fileName)) {
            // Is this a lower-weight pattern than the last match? Skip this match then.
            if (weight < results.weight)
                continue;
            bool replace = weight > results.weight;
            if (!replace) {
                // Compare the length of the match
                if (pattern.length() < results.matchingPatternLength)
                    continue; // too short, ignore
                else if (pattern.length() > results.matchingPatternLength) {
                    // longer: clear any previous match (like *.bz2, when pattern is *.tar.bz2)
                    replace = true;
                }
            }
            if (replace) {
                results.matchingMimeTypes.clear();
                // remember the new "longer" length
                results.matchingPatternLength = pattern.length();
                results.weight = weight;
            }
            results.matchingMimeTypes.append(QLatin1String(mimeType));
            if (pattern.startsWith(QLatin1String("*.")))
                results.foundSuffix = pattern.mid(2);
        }
    }
}

void QMimeBinaryProvider::matchSuffixTree(GlobMatchResult& result, QMimeBinaryProvider::CacheFile *cacheFile, int numEntries, int firstOffset, const QString &fileName)
{
    for (int i = 0; i < numEntries; ++i) {
        const int off = firstOffset + 12 * i;
        const int ch = cacheFile->getUint32(off);
        //qDebug() << QChar(ch);
        // TODO finish
    }
}

void QMimeBinaryProvider::ensureMagicLoaded()
{
}

QMimeXMLProvider::QMimeXMLProvider(QMimeDatabasePrivate *db)
    : QMimeProviderBase(db), m_loaded(false)
{
}

bool QMimeXMLProvider::isValid()
{
    return true;
}

void QMimeXMLProvider::ensureTypesLoaded()
{
    ensureLoaded();
}

QStringList QMimeXMLProvider::findByName(const QString &fileName, QString *foundSuffix)
{
    ensureLoaded();

    const QStringList matchingMimeTypes = m_mimeTypeGlobs.matchingGlobs(fileName, foundSuffix);
    return matchingMimeTypes;
}

void QMimeXMLProvider::ensureMagicLoaded()
{
    ensureLoaded();
}

void QMimeXMLProvider::ensureLoaded()
{
    if (!m_loaded) {
        m_loaded = true;

        bool fdoXmlFound = false;
        QStringList allFiles;

        const QStringList packageDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QLatin1String("mime/packages"), QStandardPaths::LocateDirectory);
        foreach (const QString &packageDir, packageDirs) {
            QDir dir(packageDir);
            const QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
            qDebug() << packageDir << files;
            if (!fdoXmlFound)
                fdoXmlFound = files.contains(QLatin1String("freedesktop.org.xml"));
            foreach (const QString& file, files) {
                allFiles.append(packageDir + QLatin1Char('/') + file);
            }
        }

        if (!fdoXmlFound) {
            // TODO: putting the xml file in the resource is a hack for now
            // We should instead install the file as part of installing Qt
            load(QLatin1String(":/qmime/freedesktop.org.xml"));
        }

        foreach (const QString& file, allFiles)
            load(file);
    }
}

void QMimeXMLProvider::load(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("QMimeDatabase: Error loading %s\n%s", qPrintable(fileName), qPrintable(file.errorString()));
        return;
    }

    QString errorMessage;
    MimeTypeParser parser(*this);
    if (!parser.parse(&file, fileName, &errorMessage))
        qWarning("QMimeDatabase: Error loading %s\n%s", qPrintable(fileName), qPrintable(errorMessage));
}

void QMimeXMLProvider::addGlobPattern(const QMimeGlobPattern& glob)
{
    m_mimeTypeGlobs.addGlob(glob);
}

bool QMimeXMLProvider::addMimeType(const QMimeType &mt)
{
    // HACK FOR NOW. The goal is to move all that code here.
    return m_db->addMimeType(mt);
}

