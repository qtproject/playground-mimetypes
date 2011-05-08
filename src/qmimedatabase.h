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

#ifndef QMIMEDATABASE_H
#define QMIMEDATABASE_H

#include "qmime_global.h"

#include "qmimetype.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMutex>

QT_BEGIN_NAMESPACE

class QByteArray;
class QFileInfo;
class QIODevice;
class QDebug;

class QMimeDatabasePrivate;
class QMIME_EXPORT QMimeDatabase : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QMimeDatabase)
public:
    typedef IMagicMatcher::IMagicMatcherList IMagicMatcherList;
    typedef IMagicMatcher::IMagicMatcherSharedPointer IMagicMatcherSharedPointer;

    explicit QMimeDatabase(QObject *parent = 0);
    ~QMimeDatabase();

    bool addMimeTypes(const QString &fileName, QString *errorMessage);
    bool addMimeTypes(QIODevice *device, QString *errorMessage);
    bool addMimeType(const QMimeType &mt);

    QMimeType findByType(const QString &type) const;
    QMimeType findByFile(const QFileInfo &fileInfo) const;
    QMimeType findByData(const QByteArray &data) const;

    // Convenience that mutex-locks the DB and calls a function
    // of the signature 'void f(const MimeType &, const QFileInfo &, const QString &)'
    // for each filename of a sequence. This avoids locking the DB for each
    // single file.
    template <class Iterator, typename Function>
    inline void findByFile(Iterator i1, const Iterator &i2, Function f) const;

    // Return all known suffixes
    QStringList suffixes() const;
    bool setPreferredSuffix(const QString &typeOrAlias, const QString &suffix);
    QString preferredSuffixByType(const QString &type) const;
    QString preferredSuffixByFile(const QFileInfo &f) const;

    QStringList filterStrings() const;
    // Return a string with all the possible file filters, for use with file dialogs
    QString allFiltersString(QString *allFilesFilter = 0) const;

    QList<MimeGlobPattern> globPatterns() const;
    void setGlobPatterns(const QString &typeOrAlias, const QList<MimeGlobPattern> &globPatterns);

    IMagicMatcherList magicMatchers() const;
    void setMagicMatchers(const QString &typeOrAlias, const IMagicMatcherList &matchers);

    QList<QMimeType> mimeTypes() const;

    // The mime types from the functions bellow are considered only in regard to
    // their glob patterns and rule-based magic matchers.
    void syncUserModifiedMimeTypes();
    static QList<QMimeType> readUserModifiedMimeTypes();
    static void writeUserModifiedMimeTypes(const QList<QMimeType> &mimeTypes);
    void clearUserModifiedMimeTypes();

    static QList<MimeGlobPattern> toGlobPatterns(const QStringList &patterns,
                                                 int weight = MimeGlobPattern::MaxWeight);
    static QStringList fromGlobPatterns(const QList<MimeGlobPattern> &globPatterns);

    friend QDebug operator<<(QDebug d, const QMimeDatabase &mt);

private:
    QMimeType findByFileUnlocked(const QFileInfo &f) const;

    QMimeDatabasePrivate *m_d;
    mutable QMutex m_mutex;
};

template <class Iterator, typename Function>
    void QMimeDatabase::findByFile(Iterator i1, const Iterator &i2, Function f) const
{
    m_mutex.lock();
    for ( ; i1 != i2; ++i1) {
        const QFileInfo fi(*i1);
        f(findByFileUnlocked(fi), fi, *i1);
    }
    m_mutex.unlock();
}

QT_END_NAMESPACE

#endif // QMIMEDATABASE_H
