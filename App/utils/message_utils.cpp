#include "message_utils.h"

namespace MessageUtils
{
void appendUniqueMessage(QStringList& messages, const QString& message)
{
    const QStringList parts = message.split('\n', Qt::SkipEmptyParts);
    for (const QString& part : parts)
    {
        const QString trimmed = part.trimmed();
        if (!trimmed.isEmpty() && !messages.contains(trimmed))
            messages.push_back(trimmed);
    }
}

QString joinMessages(const QStringList& messages)
{
    return messages.join("\n");
}

void appendMessage(QString& target, const QString& message)
{
    const QString trimmed = message.trimmed();
    if (trimmed.isEmpty())
        return;

    if (target.trimmed().isEmpty())
    {
        target = trimmed;
        return;
    }

    target += "\n" + trimmed;
}
}
