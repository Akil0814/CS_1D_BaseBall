#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QSqlDatabase>
#include <QSet>
#include <QString>

class Application;

class DatabaseManager
{
    friend class Application;

public:

    [[nodiscard]] bool init();
    [[nodiscard]] bool resetDatabase(bool remove_backup_if_success = false);

    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] bool isStadiumsAvailable() const;
    [[nodiscard]] bool isSouvenirsAvailable() const;
    [[nodiscard]] bool isDistanceAvailable() const;
    [[nodiscard]] const QString lastError() const;

    QSqlDatabase getDatabaseObj() const;


private:

    bool init_impl(bool allow_auto_recover);
    bool open_db();
    bool init_pragmas();
    bool init_schema();
    bool validate_schema_compatibility();
    bool seed_if_empty();
    bool load_table_columns(QSqlDatabase& db, const QString& table_name, QSet<QString>& out_columns, QString& out_error) const;
    bool upsert_default_souvenirs_for_all_stadiums(QSqlDatabase& db, QString& out_error);

    bool import_stadium_from_csv_files(const QString& stadium_csv);
    bool import_distances_csv_file(const QString& distances_csv);

private:
    DatabaseManager();
    DatabaseManager(const DatabaseManager& copy) = delete;
    DatabaseManager& operator=(const DatabaseManager& copy) = delete;
    DatabaseManager(DatabaseManager&& move) = delete;
    DatabaseManager& operator=(DatabaseManager&& move) = delete;

private:

    bool _initialize_all = false;

    bool _init_stadiums = false;
    bool _init_souvenirs = false;
    bool _init_distances = false;

    QString _conn_name = "main";
    QString _db_path;
    mutable QString _last_error;
};

#endif // DATABASE_MANAGER_H
