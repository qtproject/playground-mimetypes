#ifndef QMIMEMAGICRULE_H
#define QMIMEMAGICRULE_H

#include "qglobal.h"
#include <QPair>

QT_BEGIN_NAMESPACE

class QMimeMagicRulePrivate;
class QMimeMagicRule
{
public:
    enum Type {String, Byte, Big16, Big32, Little16, Little32, Host16, Host32};

    QMimeMagicRule(Type type, const QString &value, int startPos, int endPos);
    ~QMimeMagicRule();
    QMimeMagicRule(const QMimeMagicRule &other);
    QMimeMagicRule & operator=(const QMimeMagicRule &other);

    Type type() const;
    QString value() const;

    int startPos() const;
    int endPos() const;

    bool matches(const QByteArray &data) const;

    static QString toOffset(const QPair<int, int> &startEnd);
    static QPair<int, int> fromOffset(const QString &offset);

private:
    QMimeMagicRulePrivate *m_d;
};

QT_END_NAMESPACE

#endif // QMIMEMAGICRULE_H
