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
    // Initialize the database and required data.
    [[nodiscard]] bool init();

    // Rebuild the database from scratch with optional backup cleanup.
    [[nodiscard]] bool resetDatabase(bool remove_backup_if_success = false);

    // Return whether the database connection is currently open.
    [[nodiscard]] bool isOpen() const;

    // Return whether the stadium module is available.
    [[nodiscard]] bool isStadiumModuleAvailable() const;

    // Return whether the souvenir module is available.
    [[nodiscard]] bool isSouvenirModuleAvailable() const;

    // Return whether the distance module is available.
    [[nodiscard]] bool isDistanceModuleAvailable() const;

    // Return the active database object for internal use.
    [[nodiscard]] QSqlDatabase getDatabaseObj() const;

    // Return the last recorded error message.
    [[nodiscard]] const QString lastError() const;

    // Return whether an error message is currently stored.
    [[nodiscard]] bool hasError() const;

    // Return the last recorded warning message.
    const QString lastWarning() const;

    // Return whether a warning message is currently stored.
    bool hasWarning() const;

private:

    bool init_impl(bool allow_auto_recover);
    bool is_table_schema_compatible();

    bool open_db();
    bool init_pragmas();
    bool init_schema();
    bool seed_if_empty();
    
    bool upsert_default_souvenirs_for_all_stadiums(QSqlDatabase& db);

    bool import_stadium_from_csv_files(const QString& stadium_csv);
    bool import_distances_csv_file(const QString& distances_csv);

private:
    DatabaseManager();
    DatabaseManager(const DatabaseManager& copy) = delete;
    DatabaseManager& operator=(const DatabaseManager& copy) = delete;
    DatabaseManager(DatabaseManager&& move) = delete;
    DatabaseManager& operator=(DatabaseManager&& move) = delete;

private:

    bool  _core_initialized = false;
    bool  _fully_initialized = false;


    bool _schema_stadiums_ready = false;
    bool _schema_souvenirs_ready = false;
    bool _schema_distances_ready = false;

    bool _data_stadiums_ready = false;
    bool _data_souvenirs_ready = false;
    bool _data_distances_ready = false;

    QString _conn_name = "main";
    QString _db_path;
    QString _last_error;
    QString _last_warning;
};

#endif // DATABASE_MANAGER_H
