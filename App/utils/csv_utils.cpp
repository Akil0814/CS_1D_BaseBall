#include "csv_utils.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

namespace CsvUtils
{
QString normalizeKey(const QString& input)
{
    QString out;
    out.reserve(input.size());

    bool last_was_sep = false;
    for (const QChar ch : input.trimmed().toLower())
    {
        if (ch.isLetterOrNumber())
        {
            out.append(ch);
            last_was_sep = false;
            continue;
        }

        if (!last_was_sep)
        {
            out.append('_');
            last_was_sep = true;
        }
    }

    while (out.startsWith('_'))
        out.remove(0, 1);
    while (out.endsWith('_'))
        out.chop(1);

    return out;
}

QString normalizeStadiumNameKey(const QString& input)
{
    QString out = input;
    out.replace(QChar(0x2013), '-'); // en dash
    out.replace(QChar(0x2014), '-'); // em dash
    out.replace(QChar(0x00A0), ' '); // nbsp

    return out.simplified().toLower();
}

QStringList parseLine(const QString& line)
{
    QStringList fields;
    QString current;
    bool in_quotes = false;

    for (int i = 0; i < line.size(); ++i)
    {
        const QChar ch = line[i];
        if (ch == '"')
        {
            if (in_quotes && i + 1 < line.size() && line[i + 1] == '"')
            {
                current.append('"');
                ++i;
            }
            else
            {
                in_quotes = !in_quotes;
            }
            continue;
        }

        if (ch == ',' && !in_quotes)
        {
            fields.push_back(current.trimmed());
            current.clear();
            continue;
        }

        current.append(ch);
    }

    fields.push_back(current.trimmed());
    return fields;
}

bool readTable(const QString& file_path, Table& out_table, QString& out_error)
{
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        out_error = "Cannot open csv file: " + file_path;
        return false;
    }

    QTextStream in(&file);
    bool header_set = false;

    while (!in.atEnd())
    {
        const QString raw_line = in.readLine();
        const QStringList row = parseLine(raw_line);

        bool all_empty = true;
        for (const QString& cell : row)
        {
            if (!cell.trimmed().isEmpty())
            {
                all_empty = false;
                break;
            }
        }
        if (all_empty)
            continue;

        if (!header_set)
        {
            out_table.header = row;
            if (!out_table.header.isEmpty())
                out_table.header[0].remove(QChar(0xFEFF)); // UTF-8 BOM

            header_set = true;
            continue;
        }

        QStringList normalized_row = row;
        while (normalized_row.size() < out_table.header.size())
            normalized_row.push_back(QString());

        out_table.rows.push_back(normalized_row);
    }

    if (!header_set)
    {
        out_error = "CSV has no header: " + file_path;
        return false;
    }

    return true;
}

QHash<QString, int> buildHeaderIndex(const QStringList& header)
{
    QHash<QString, int> index_by_key;
    for (int i = 0; i < header.size(); ++i)
    {
        const QString key = normalizeKey(header[i]);
        if (!key.isEmpty() && !index_by_key.contains(key))
            index_by_key.insert(key, i);
    }
    return index_by_key;
}

int findHeaderIndex(const QHash<QString, int>& header_index, const QVector<QString>& candidates)
{
    for (const QString& candidate : candidates)
    {
        const QString key = normalizeKey(candidate);
        if (header_index.contains(key))
            return header_index.value(key);
    }

    return -1;
}

QString cellAt(const QStringList& row, int index)
{
    if (index < 0 || index >= row.size())
        return QString();
    return row[index].trimmed();
}

int parseIntLoose(const QString& text, int default_value)
{
    QString cleaned;
    cleaned.reserve(text.size());
    for (const QChar ch : text)
    {
        if (ch.isDigit() || ch == '-')
            cleaned.append(ch);
    }

    bool ok = false;
    const int value = cleaned.toInt(&ok);
    return ok ? value : default_value;
}

double parseFirstNumber(const QString& text, double default_value)
{
    static const QRegularExpression number_re(R"([-+]?\d*\.?\d+)");
    const QRegularExpressionMatch match = number_re.match(text);
    if (!match.hasMatch())
        return default_value;

    bool ok = false;
    const double value = match.captured(0).toDouble(&ok);
    return ok ? value : default_value;
}
}
