#ifndef MESSAGE_UTILS_H
#define MESSAGE_UTILS_H

#include <QString>
#include <QStringList>

namespace MessageUtils
{
void appendUniqueMessage(QStringList& messages, const QString& message);
QString joinMessages(const QStringList& messages);
void appendMessage(QString& target, const QString& message);
}

#endif // MESSAGE_UTILS_H
