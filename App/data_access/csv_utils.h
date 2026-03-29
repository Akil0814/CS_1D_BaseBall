#ifndef CSV_UTILS_H
#define CSV_UTILS_H

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

namespace CsvUtils
{
struct Table
{
    QStringList header;
    QVector<QStringList> rows;
};

QString normalizeKey(const QString& input);
QString normalizeStadiumNameKey(const QString& input);

QStringList parseLine(const QString& line);
bool readTable(const QString& file_path, Table& out_table, QString& out_error);

QHash<QString, int> buildHeaderIndex(const QStringList& header);
int findHeaderIndex(const QHash<QString, int>& header_index, const QVector<QString>& candidates);

QString cellAt(const QStringList& row, int index);
int parseIntLoose(const QString& text, int default_value = 0);
double parseFirstNumber(const QString& text, double default_value = 0.0);
}

#endif // CSV_UTILS_H
