#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QSqlDatabase>
#include <QString>

class Application;

class DatabaseManager
{
    friend class Application;

public:

    [[nodiscard]] bool init();
    [[nodiscard]] bool resetDatabase(bool remove_backup_if_success = false);

    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] const QString lastError() const;

    QSqlDatabase getDatabaseObj() const;


private:

    bool open_db();
    bool init_pragmas();
    bool init_schema();
    bool seed_if_empty();

    bool import_stadium_from_csv_files(const QString& stadium_csv);
    bool import_distances_csv_file(const QString& distances_csv);

private:
    DatabaseManager();
    DatabaseManager(const DatabaseManager& copy) = delete;
    DatabaseManager& operator=(const DatabaseManager& copy) = delete;
    DatabaseManager(DatabaseManager&& move) = delete;
    DatabaseManager& operator=(DatabaseManager&& move) = delete;

private:

    bool _initialize = false;
    QString _conn_name = "main";
    QString _db_path;
    mutable QString _last_error;
};

#endif // DATABASE_MANAGER_H
