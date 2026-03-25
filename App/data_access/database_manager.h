#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include<iostream>
#include <algorithm>
#include <functional>

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QStandardPaths>
#include <QSqlError>
#include <QSqlQuery>
#include <QVector>
#include <optional>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QDir>
#include <QSet>
#include <QHash>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

class Application;

class DatabaseManager
{
    friend class Application;

public:

    [[nodiscard]] bool init();
    [[nodiscard]] bool resetDatabase(bool remove_backup_if_success = false);

    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] const std::string lastError();


private:
    bool open_db();
    bool init_pragmas();
    bool init_schema();
    bool seed_if_empty();
    bool import_from_csv_files(const QString& souvenirs_csv, const QString& distances_csv);
    bool import_distances_csv_file(const QString& distances_csv);

private:
    DatabaseManager();
    DatabaseManager(const DatabaseManager& copy) = delete;
    DatabaseManager& operator=(const DatabaseManager& copy) = delete;
    DatabaseManager(DatabaseManager&& move) = delete;
    DatabaseManager& operator=(DatabaseManager&& move) = delete;

private:

    bool _initialize = false;
};

#endif // DATABASE_MANAGER_H
