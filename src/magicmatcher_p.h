#ifndef MAGICMATCHER_P_H
#define MAGICMATCHER_P_H

#include "magicmatcher.h"
#include "qmimetype.h"

#include <QtCore/QFileInfo>

QT_BEGIN_NAMESPACE

class FileMatchContext
{
    Q_DISABLE_COPY(FileMatchContext)

public:
    // Max data to be read from a file
    enum { MaxData = 2500 };

    explicit FileMatchContext(const QFileInfo &fi);

    inline QString fileName() const { return m_fileName; }
    // Return (cached) first MaxData bytes of file
    QByteArray data();

private:
    const QFileInfo m_fileInfo;
    const QString m_fileName;
    enum State {
        // File cannot be read/does not exist
        NoDataAvailable,
        // Not read yet
        DataNotRead,
        // Available
        DataRead
    } m_state;
    QByteArray m_data;
};

QT_END_NAMESPACE

#endif // MAGICMATCHER_P_H
