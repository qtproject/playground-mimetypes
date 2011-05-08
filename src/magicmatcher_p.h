#ifndef MAGICMATCHER_P_H
#define MAGICMATCHER_P_H

#include "magicmatcher.h"
#include "qmimetype.h"

#include <QtCore/QFileInfo>

// Fallback priorities, must be low.
enum {
    BinaryMatchPriority = MimeGlobPattern::MinWeight + 1,
    TextMatchPriority
};

typedef QSharedPointer<MagicRuleMatcher> MagicRuleMatcherPtr;

//namespace Internal {

class FileMatchContext {
    Q_DISABLE_COPY(FileMatchContext)
public:
    // Max data to be read from a file
    enum { MaxData = 2048 };

    explicit FileMatchContext(const QFileInfo &fi);

    inline QString fileName() const { return m_fileName; }
    // Return (cached) first MaxData bytes of file
    QByteArray data();

private:
    enum State {
        // File cannot be read/does not exist
        NoDataAvailable,
        // Not read yet
        DataNotRead,
        // Available
        DataRead };
    const QFileInfo m_fileInfo;
    const QString m_fileName;
    State m_state;
    QByteArray m_data;
};

//} // namespace Internal

//namespace Internal {

class BinaryMatcher : public IMagicMatcher {
    Q_DISABLE_COPY(BinaryMatcher)
public:
    BinaryMatcher() {}
    virtual bool matches(const QByteArray & /*data*/) const { return true; }
    virtual int priority() const  { return BinaryMatchPriority; }
};

class HeuristicTextMagicMatcher : public IMagicMatcher {
    Q_DISABLE_COPY(HeuristicTextMagicMatcher)
public:
    HeuristicTextMagicMatcher() {}
    virtual bool matches(const QByteArray &data) const;
    virtual int priority() const  { return TextMatchPriority; }

    static bool isTextFile(const QByteArray &data);
};

//} // namespace Internal

#endif // MAGICMATCHER_P_H
