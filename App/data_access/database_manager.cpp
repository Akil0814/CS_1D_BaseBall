#include "database_manager.h"
#include "../utils/message_utils.h"
#include "../utils/csv_utils.h"

#include <algorithm>
#include <iostream>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QSqlError>
#include <QSqlQuery>
#include <QSet>
#include <QStringList>
#include <QVector>

namespace
{
struct DefaultSouvenir
{
    const char* name;
    double price;
};

const QVector<DefaultSouvenir> k_default_souvenirs =
{
    { "Baseball cap", 19.99 },
    { "Baseball bat", 89.39 },
    { "Team pennant", 17.99 },
    { "Autographed baseball", 29.99 },
    { "Team jersey", 199.99 }
};

QString normalizeIdentityValue(const QString& value)
{
    return CsvUtils::normalizeStadiumNameKey(value);
}

}

//-------------------------------------public--------------------------------------

DatabaseManager::DatabaseManager(){}

bool DatabaseManager::init()
{
    return init_impl(true);
}

void DatabaseManager::setDatabasePath(const QString& db_path)
{
    _db_path = db_path;
}

bool DatabaseManager::resetDatabase(bool remove_backup_if_success)
{
    _last_error.clear();
    _last_warning.clear();

    _core_initialized = false;
    _fully_initialized = false;

    _schema_stadiums_ready = false;
    _schema_souvenirs_ready = false;
    _schema_distances_ready = false;

    _data_stadiums_ready = false;
    _data_souvenirs_ready = false;
    _data_distances_ready = false;

    auto close_connection = [&]()
        {
            if (!QSqlDatabase::contains(_conn_name))
                return;

            {
                QSqlDatabase db = QSqlDatabase::database(_conn_name, false);
                if (db.isValid() && db.isOpen())
                    db.close();
            }
            QSqlDatabase::removeDatabase(_conn_name);
        };

    close_connection();

    QString time = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");
    const QString backup_path = _db_path + "." + time + ".bak";
    bool has_backup = false;

    if (QFile::exists(_db_path))
    {
        QFile db_file(_db_path);
        if (!db_file.rename(backup_path))
        {
            const QString backup_error_detail = db_file.errorString().trimmed();
            const bool database_usable = init_impl(false);
            const QString retry_error = _last_error.trimmed();

            _last_error = "failed to back up database file";
            MessageUtils::appendMessage(_last_error, backup_error_detail);
            MessageUtils::appendMessage(_last_error, database_usable
                ? "database remains usable after retry"
                : "database is not usable after retry");
            MessageUtils::appendMessage(_last_error, retry_error);
            _core_initialized = false;
            _fully_initialized = false;

            return false;
        }

        has_backup = true;
    }


    if (!init_impl(false))
    {
        const QString new_database_error = _last_error.trimmed();
        const QString new_database_warning = _last_warning.trimmed();

        close_connection();
        QFile::remove(_db_path);
        bool restored = false;
        QString restore_error_detail;
        if (has_backup)
        {
            QFile backup_file(backup_path);
            restored = backup_file.rename(_db_path);
            if (!restored)
                restore_error_detail = backup_file.errorString().trimmed();
        }

        _core_initialized = false;
        _fully_initialized = false;
        _last_error = "failed to initialize new database";
        MessageUtils::appendMessage(_last_error, new_database_error);
        MessageUtils::appendMessage(_last_warning, new_database_warning);

        if (has_backup && !restored)
        {
            MessageUtils::appendMessage(_last_error, "failed to restore backup file");
            MessageUtils::appendMessage(_last_error, restore_error_detail);
        }

        const bool recovered = init_impl(false);
        const QString recovery_error = _last_error.trimmed();
        const QString recovery_warning = _last_warning.trimmed();

        _last_error = "failed to initialize new database";
        MessageUtils::appendMessage(_last_error, new_database_error);
        if (has_backup && !restored)
        {
            MessageUtils::appendMessage(_last_error, "failed to restore backup file");
            MessageUtils::appendMessage(_last_error, restore_error_detail);
        }
        MessageUtils::appendMessage(_last_error, recovered
            ? (restored ? "restored backup successfully" : "database is usable after retry")
            : (restored ? "failed after restoring backup" : "failed to recover database"));
        MessageUtils::appendMessage(_last_error, recovery_error);

        QStringList warnings;
        MessageUtils::appendUniqueMessage(warnings, new_database_warning);
        MessageUtils::appendUniqueMessage(warnings, recovery_warning);
        _last_warning = MessageUtils::joinMessages(warnings);

        return false;
    }

    if (remove_backup_if_success && has_backup)
        QFile::remove(backup_path);

    return true;
}

bool DatabaseManager::isOpen() const
{
    if (!QSqlDatabase::contains(_conn_name))
        return false;

    const QSqlDatabase db = QSqlDatabase::database(_conn_name, false);

    return db.isValid() && db.isOpen();
}

bool DatabaseManager::isStadiumModuleAvailable() const
{
    return _schema_stadiums_ready && _data_stadiums_ready;
}

bool DatabaseManager::isSouvenirModuleAvailable() const
{
    return _schema_souvenirs_ready && _data_souvenirs_ready;
}

bool DatabaseManager::isDistanceModuleAvailable() const
{
    return _schema_distances_ready && _data_distances_ready;
}

const QString DatabaseManager::lastError() const
{
	return _last_error;
}

const QString DatabaseManager::lastWarning() const
{
    return _last_warning;
}

bool DatabaseManager::hasError() const
{
    return !_last_error.trimmed().isEmpty();
}

bool DatabaseManager::hasWarning() const
{
    return !_last_warning.trimmed().isEmpty();
}

QSqlDatabase DatabaseManager::getDatabaseObj()
{
    _last_error.clear();

    if (_conn_name.isEmpty())
    {
        _last_error = "Database connection name is empty.";
        return QSqlDatabase();
    }

    if (!QSqlDatabase::contains(_conn_name))
    {
        _last_error = "Database connection does not exist: " + _conn_name;
        return QSqlDatabase();
    }

    QSqlDatabase db = QSqlDatabase::database(_conn_name, false);

    if (!db.isValid())
    {
        _last_error = "Database connection is invalid: " + _conn_name;
        return QSqlDatabase();
    }

    if (!db.isOpen())
    {
        _last_error = "Database connection is not open: " + _conn_name;
        return QSqlDatabase();
    }

    return db;
}

bool DatabaseManager::importStadiumsFromFile(const QString& filePath)
{
    _last_error.clear();
    _last_warning.clear();

    QStringList warnings;
    const QString import_target = QFileInfo(filePath).fileName().trimmed().isEmpty()
        ? filePath
        : QFileInfo(filePath).fileName();
    auto set_failure_warning = [&](const QString& message)
        {
            QStringList failure_warnings;
            MessageUtils::appendUniqueMessage(failure_warnings, message);
            MessageUtils::appendUniqueMessage(failure_warnings, _last_warning);
            _last_warning = MessageUtils::joinMessages(failure_warnings);
        };

    if (!_schema_stadiums_ready)
    {
        _last_error = "Stadium table is unavailable; cannot import stadium data.";
        set_failure_warning("stadium import failed for file: " + import_target);
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(_conn_name, false);
    if (!db.isValid() || !db.isOpen())
    {
        _last_error = "Database is not open.";
        set_failure_warning("stadium import failed for file: " + import_target);
        return false;
    }

    QSet<QString> existing_team_keys;
    QSet<QString> existing_stadium_name_keys;
    {
        QSqlQuery stadium_q(db);
        if (!stadium_q.exec("SELECT team_name, stadium_name FROM stadiums;"))
        {
            _last_error = "load stadium snapshot for import failed: " + stadium_q.lastError().text();
            set_failure_warning("stadium import failed for file: " + import_target);
            return false;
        }

        while (stadium_q.next())
        {
            existing_team_keys.insert(normalizeIdentityValue(stadium_q.value(0).toString()));
            existing_stadium_name_keys.insert(normalizeIdentityValue(stadium_q.value(1).toString()));
        }
    }

    if (!import_stadium_from_csv_files(filePath))
    {
        set_failure_warning("stadium import failed for file: " + import_target);
        return false;
    }

    _data_stadiums_ready = true;

    if (!_schema_souvenirs_ready)
        return true;

    CsvUtils::Table csv;
    QString csv_error;
    if (!CsvUtils::readTable(filePath, csv, csv_error))
    {
        MessageUtils::appendUniqueMessage(
            warnings,
            "stadium import completed but default souvenir backfill could not read csv: " + csv_error);
        _last_warning = MessageUtils::joinMessages(warnings);
        return true;
    }

    const QHash<QString, int> header_index = CsvUtils::buildHeaderIndex(csv.header);
    const int team_idx = CsvUtils::findHeaderIndex(header_index, { "team_name", "team" });
    const int stadium_idx = CsvUtils::findHeaderIndex(header_index, { "stadium_name", "stadium" });

    if (team_idx < 0 || stadium_idx < 0)
    {
        MessageUtils::appendUniqueMessage(
            warnings,
            "stadium import completed but default souvenir backfill skipped: csv header missing team/stadium columns.");
        _last_warning = MessageUtils::joinMessages(warnings);
        return true;
    }

    QVector<int> new_stadium_ids;
    QSet<int> seen_new_stadium_ids;

    for (const QStringList& row : csv.rows)
    {
        const QString team_name = CsvUtils::cellAt(row, team_idx).trimmed();
        const QString stadium_name = CsvUtils::cellAt(row, stadium_idx).trimmed();

        if (team_name.isEmpty() || stadium_name.isEmpty())
            continue;

        const QString team_key = normalizeIdentityValue(team_name);
        const QString stadium_key = normalizeIdentityValue(stadium_name);

        if (existing_team_keys.contains(team_key) || existing_stadium_name_keys.contains(stadium_key))
            continue;

        QSqlQuery lookup_q(db);
        lookup_q.prepare(R"SQL(
            SELECT stadium_id
            FROM stadiums
            WHERE team_name = ? OR stadium_name = ?
            ORDER BY stadium_id ASC
            LIMIT 1;
        )SQL");
        lookup_q.addBindValue(team_name);
        lookup_q.addBindValue(stadium_name);

        if (!lookup_q.exec())
        {
            MessageUtils::appendUniqueMessage(
                warnings,
                "stadium import completed but failed to resolve new stadium id for souvenir backfill: "
                + lookup_q.lastError().text());
            continue;
        }

        if (!lookup_q.next())
            continue;

        const int stadium_id = lookup_q.value(0).toInt();
        if (stadium_id <= 0 || seen_new_stadium_ids.contains(stadium_id))
            continue;

        seen_new_stadium_ids.insert(stadium_id);
        new_stadium_ids.push_back(stadium_id);
    }

    if (new_stadium_ids.isEmpty())
    {
        _data_souvenirs_ready = true;
        return true;
    }

    if (!upsert_default_souvenirs_for_stadium_ids(db, new_stadium_ids))
    {
        MessageUtils::appendUniqueMessage(
            warnings,
            "stadium import completed but default souvenir backfill failed: " + _last_error);
        _last_warning = MessageUtils::joinMessages(warnings);
        _last_error.clear();
        return true;
    }

    _last_warning = MessageUtils::joinMessages(warnings);
    _data_souvenirs_ready = true;
    return true;
}

bool DatabaseManager::importDistancesFromFile(const QString& filePath)
{
    _last_error.clear();
    _last_warning.clear();

    const QString import_target = QFileInfo(filePath).fileName().trimmed().isEmpty()
        ? filePath
        : QFileInfo(filePath).fileName();
    auto set_failure_warning = [&](const QString& message)
        {
            QStringList failure_warnings;
            MessageUtils::appendUniqueMessage(failure_warnings, message);
            MessageUtils::appendUniqueMessage(failure_warnings, _last_warning);
            _last_warning = MessageUtils::joinMessages(failure_warnings);
        };

    if (!_schema_stadiums_ready || !_data_stadiums_ready)
    {
        _last_error = "Stadium data is unavailable; cannot import distance data.";
        set_failure_warning("distance import failed for file: " + import_target);
        return false;
    }

    if (!_schema_distances_ready)
    {
        _last_error = "Distance table is unavailable; cannot import distance data.";
        set_failure_warning("distance import failed for file: " + import_target);
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(_conn_name, false);
    if (!db.isValid() || !db.isOpen())
    {
        _last_error = "Database is not open.";
        set_failure_warning("distance import failed for file: " + import_target);
        return false;
    }

    if (!import_distances_csv_file(filePath))
    {
        set_failure_warning("distance import failed for file: " + import_target);
        return false;
    }

    _data_distances_ready = true;
    return true;
}

//-------------------------------------private--------------------------------------

bool DatabaseManager::init_impl(bool allow_auto_recover)
{
    if (_fully_initialized)
        return true;

    _last_error.clear();
    _last_warning.clear();

    _schema_stadiums_ready = false;
    _schema_souvenirs_ready = false;
    _schema_distances_ready = false;

    _data_stadiums_ready = false;
    _data_souvenirs_ready = false;
    _data_distances_ready = false;

    if (!open_db())
        return false;

    if (!init_pragmas())
        return false;

    if (allow_auto_recover == true)
    {
        const SchemaCheckResult schema_check_result = check_table_schema_compatible();

        if (schema_check_result == SchemaCheckResult::Incompatible)
        {
            if (!resetDatabase(false))
                return false;
            return true;
        }

        if (schema_check_result == SchemaCheckResult::CheckFailed)
            return false;
    }

    if(!init_schema())
        return false;

    if (!seed_if_empty())
        return false;

    _core_initialized = (_schema_stadiums_ready && _data_stadiums_ready);

    if(_schema_souvenirs_ready&& _data_souvenirs_ready &&
        _schema_distances_ready&& _data_distances_ready)
        _fully_initialized = true;

    return true;
}

bool DatabaseManager::open_db()
{
    _last_error.clear();
    _last_warning.clear();

    if (_db_path.trimmed().isEmpty())
    {
        _last_error = "Database path is not configured.";
        return false;
    }

    QSqlDatabase db = QSqlDatabase::contains(_conn_name)
        ? QSqlDatabase::database(_conn_name)
        : QSqlDatabase::addDatabase("QSQLITE", _conn_name);

    db.setConnectOptions("QSQLITE_BUSY_TIMEOUT=5000");
    db.setDatabaseName(_db_path);

    if (!db.open())
    {
        _last_error = db.lastError().text();
        return false;
    }

    std::cout << " " << _db_path.toStdString() << std::endl;

    return true;
}

bool DatabaseManager::init_pragmas()
{
    _last_error.clear();
    _last_warning.clear();

    QSqlDatabase db = QSqlDatabase::database(_conn_name);
    if (!db.isValid() || !db.isOpen())
    {
        _last_error = "Database is not open.";
        return false;
    }

    QSqlQuery q(db);

    auto exec_sql = [&](const QString& sql) -> bool
        {
            if (!q.exec(sql))
            {
                _last_error = q.lastError().text() + " | SQL: " + sql;
                return false;
            }
            return true;
        };

    if (!exec_sql("PRAGMA foreign_keys = ON;"))
        return false;

    if (!q.exec("PRAGMA journal_mode = WAL;"))
    {
        if (!q.exec("PRAGMA journal_mode = DELETE;"))
        {
            _last_error = q.lastError().text() + " | SQL: PRAGMA journal_mode";
            return false;
        }
    }

    if (!exec_sql("PRAGMA synchronous = NORMAL;"))
        return false;

    if (!exec_sql("PRAGMA temp_store = MEMORY;"))
        return false;

    return true;
}

bool DatabaseManager::init_schema()
{
    _last_error.clear();
    _last_warning.clear();

    _schema_stadiums_ready = false;
    _schema_souvenirs_ready = false;
    _schema_distances_ready = false;

    QSqlDatabase db = QSqlDatabase::database(_conn_name);
    if (!db.isValid() || !db.isOpen())
    {
        _last_error = "Database is not open.";
        return false;
    }

    QStringList warnings;

    QSqlQuery q(db);

    auto exec_sql = [&](const QString& sql) -> bool
        {
            if (!q.exec(sql))
            {
                _last_error = q.lastError().text() + " | SQL: " + sql;
                return false;
            }
            return true;
        };

    auto run_step_transaction = [&](const QString& step_name, const QVector<QString>& sqls) -> bool
        {
            if (!db.transaction())
            {
                _last_error = QString("[%1] %2").arg(step_name, db.lastError().text());
                return false;
            }

            for (const QString& sql : sqls)
            {
                if (!exec_sql(sql))
                {
                    _last_error = QString("[%1] %2").arg(step_name, _last_error);
                    db.rollback();
                    return false;
                }
            }

            if (!db.commit())
            {
                _last_error = QString("[%1] %2").arg(step_name, db.lastError().text());
                db.rollback();
                return false;
            }

            return true;
        };

    // Step 1: stadiums (from MLB Information*.csv)
    if (!run_step_transaction("create_stadiums",
        {
            R"SQL(
                CREATE TABLE IF NOT EXISTS stadiums
                (
                    stadium_id INTEGER PRIMARY KEY AUTOINCREMENT,
                    team_name TEXT NOT NULL COLLATE NOCASE,
                    stadium_name TEXT NOT NULL COLLATE NOCASE,
                    seating_capacity INTEGER NOT NULL DEFAULT 0 CHECK (seating_capacity >= 0),
                    location TEXT NOT NULL DEFAULT '',
                    playing_surface TEXT NOT NULL DEFAULT '',
                    league TEXT NOT NULL DEFAULT '',
                    date_opened INTEGER NOT NULL DEFAULT 0 CHECK (date_opened >= 0),
                    distance_to_center_field_ft REAL NOT NULL DEFAULT 0 CHECK (distance_to_center_field_ft >= 0),
                    distance_to_center_field_raw TEXT NOT NULL DEFAULT '',
                    ballpark_typology TEXT NOT NULL DEFAULT '',
                    roof_type TEXT NOT NULL DEFAULT '',
                    is_expansion INTEGER NOT NULL DEFAULT 0 CHECK (is_expansion IN (0, 1)),
                    UNIQUE(team_name),
                    UNIQUE(stadium_name)
                );
            )SQL",
            "CREATE INDEX IF NOT EXISTS idx_stadiums_league ON stadiums(league);",
            "CREATE INDEX IF NOT EXISTS idx_stadiums_name ON stadiums(stadium_name);"
        }))
    {
        _last_error = "stadium schema failed: " + _last_error;
        return false;
    }
    _schema_stadiums_ready = true;

    // Step 2: souvenirs
    if (!run_step_transaction("create_souvenirs",
        {
        R"SQL(
            CREATE TABLE IF NOT EXISTS souvenirs
            (
                souvenir_id INTEGER PRIMARY KEY AUTOINCREMENT,
                stadium_id INTEGER NOT NULL,
                name TEXT NOT NULL COLLATE NOCASE,
                price REAL NOT NULL DEFAULT 0 CHECK (price >= 0),
                UNIQUE(stadium_id, name),
                FOREIGN KEY(stadium_id) REFERENCES stadiums(stadium_id)
                    ON UPDATE CASCADE
                    ON DELETE CASCADE
            );
        )SQL",
        "CREATE INDEX IF NOT EXISTS idx_souvenirs_stadium_id ON souvenirs(stadium_id);"
        }))
    {
        MessageUtils::appendUniqueMessage(warnings, "souvenir schema degraded: " + _last_error);
        _last_error.clear();
        _schema_souvenirs_ready = false;
    }
    else
        _schema_souvenirs_ready = true;

    // Step 3: stadium_distances as undirected edges
    if (!run_step_transaction("create_stadium_distances",
        {
            R"SQL(
                CREATE TABLE IF NOT EXISTS stadium_distances
                (
                    stadium_a_id INTEGER NOT NULL,
                    stadium_b_id INTEGER NOT NULL,
                    distance_miles REAL NOT NULL CHECK (distance_miles >= 0),
                    PRIMARY KEY(stadium_a_id, stadium_b_id),
                    CHECK (stadium_a_id < stadium_b_id),
                    FOREIGN KEY(stadium_a_id) REFERENCES stadiums(stadium_id)
                        ON UPDATE CASCADE
                        ON DELETE CASCADE,
                    FOREIGN KEY(stadium_b_id) REFERENCES stadiums(stadium_id)
                        ON UPDATE CASCADE
                        ON DELETE CASCADE
                );
            )SQL",
            "CREATE INDEX IF NOT EXISTS idx_distances_b ON stadium_distances(stadium_b_id);"
        }))
    {
        MessageUtils::appendUniqueMessage(warnings, "distance schema degraded: " + _last_error);
        _last_error.clear();
        _schema_distances_ready = false;
    }
    else
        _schema_distances_ready = true;

    _last_warning = MessageUtils::joinMessages(warnings);

    return true;
}

bool DatabaseManager::seed_if_empty()
{
    const QString inherited_warning = _last_warning.trimmed();

    _last_error.clear();
    _last_warning = inherited_warning;

    _data_stadiums_ready = false;
    _data_souvenirs_ready = false;
    _data_distances_ready = false;

    QSqlDatabase db = QSqlDatabase::database(_conn_name);
    if (!db.isValid() || !db.isOpen())
    {
        _last_error = "Database is not open.";
        return false;
    }

    if (!_schema_stadiums_ready)
    {
        _last_error = "Stadium table is unavailable; cannot complete seed process.";
        return false;
    }

    QStringList warnings;

    auto table_row_count = [&](const QString& table_name) -> int
        {
            QSqlQuery query(db);
            if (!query.exec("SELECT COUNT(1) FROM " + table_name + ";"))
            {
                _last_error = query.lastError().text() + " | SQL: SELECT COUNT FROM " + table_name;
                return -1;
            }

            if (!query.next())
                return 0;

            return query.value(0).toInt();
        };

    // stadiums: core data
    const int stadium_count = table_row_count("stadiums");
    if (stadium_count == -1)
    {
        _last_error = "stadium data check failed: " + _last_error;
        _data_stadiums_ready = false;
        return false;
    }

    // distances: optional data
    int distance_count = -1;
    if (_schema_distances_ready)
    {
        distance_count = table_row_count("stadium_distances");
        if (distance_count == -1)
        {
            MessageUtils::appendUniqueMessage(warnings, "distance data degraded: " + _last_error);
            _last_error.clear();
            _data_distances_ready = false;
        }
    }
    else
        _data_distances_ready = false;

    const QString data_dir = QFileInfo(_db_path).absolutePath();
    const QString stadium_csv = QDir(data_dir).filePath("MLB Information.csv");
    const QString distances_csv = QDir(data_dir).filePath("Distance between stadiums.csv");

    const bool is_initial_stadium_seed = (stadium_count == 0);

    // seed stadiums if empty
    if (stadium_count == 0)
    {
        if (!import_stadium_from_csv_files(stadium_csv))
        {
            _last_error = "stadium data load failed: " + _last_error;
            _data_stadiums_ready = false;
            return false;
        }
    }
    _data_stadiums_ready = true;

    if (_schema_souvenirs_ready)
    {
        if (is_initial_stadium_seed)
        {
            if (!upsert_default_souvenirs_for_all_stadiums(db))
            {
                MessageUtils::appendUniqueMessage(warnings, "souvenir data degraded: " + _last_error);
                _last_error.clear();
                _data_souvenirs_ready = false;
            }
            else
                _data_souvenirs_ready = true;
        }
        else
            _data_souvenirs_ready = true;
    }

    // seed distances
    if (_schema_distances_ready)
    {
        if (distance_count > 0)
            _data_distances_ready = true;
        else if (distance_count == 0)
        {
            if (!import_distances_csv_file(distances_csv))
            {
                MessageUtils::appendUniqueMessage(warnings, "distance data degraded: " + _last_error);
                _last_error.clear();
                _data_distances_ready = false;
            }
            else
            {
                if (hasWarning())
                    MessageUtils::appendUniqueMessage(warnings, lastWarning());
                _data_distances_ready = true;
            }
        }
    }

    QStringList merged_warnings;
    MessageUtils::appendUniqueMessage(merged_warnings, inherited_warning);
    for (const QString& warning : warnings)
        MessageUtils::appendUniqueMessage(merged_warnings, warning);
    _last_warning = MessageUtils::joinMessages(merged_warnings);

    return _schema_stadiums_ready && _data_stadiums_ready;
}

bool DatabaseManager::upsert_default_souvenirs_for_all_stadiums(QSqlDatabase& db)
{
    QVector<int> stadium_ids;
    QSqlQuery stadium_q(db);
    if (!stadium_q.exec("SELECT stadium_id FROM stadiums;"))
    {
        _last_error =
            "load stadium ids for souvenir upsert failed: "
            + stadium_q.lastError().text()
            + " | SQL: SELECT stadium_id FROM stadiums;";
        return false;
    }

    while (stadium_q.next())
        stadium_ids.push_back(stadium_q.value(0).toInt());

    return upsert_default_souvenirs_for_stadium_ids(db, stadium_ids);
}

bool DatabaseManager::upsert_default_souvenirs_for_stadium_ids(QSqlDatabase& db, const QVector<int>& stadium_ids)
{
    if (stadium_ids.isEmpty())
        return true;

    if (!db.transaction())
    {
        _last_error = "begin souvenir upsert transaction failed: " + db.lastError().text();
        return false;
    }

    QSet<int> unique_ids;
    QSqlQuery upsert_q(db);
    upsert_q.prepare(R"SQL(
        INSERT INTO souvenirs(stadium_id, name, price)
        VALUES (?, ?, ?)
        ON CONFLICT(stadium_id, name) DO NOTHING;
    )SQL");

    for (const int stadium_id : stadium_ids)
    {
        if (stadium_id <= 0 || unique_ids.contains(stadium_id))
            continue;

        unique_ids.insert(stadium_id);

        for (const DefaultSouvenir& item : k_default_souvenirs)
        {
            upsert_q.bindValue(0, stadium_id);
            upsert_q.bindValue(1, QString::fromUtf8(item.name));
            upsert_q.bindValue(2, item.price);

            if (!upsert_q.exec())
            {
                _last_error =
                    "upsert default souvenir failed for stadium_id="
                    + QString::number(stadium_id)
                    + ", item=" + QString::fromUtf8(item.name)
                    + ": " + upsert_q.lastError().text();
                db.rollback();
                return false;
            }
        }
    }

    if (!db.commit())
    {
        _last_error = "commit souvenir upsert transaction failed: " + db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}

bool DatabaseManager::import_stadium_from_csv_files(const QString& stadium_csv)
{
    _last_error.clear();

    CsvUtils::Table csv;
    QString csv_error;
    if (!CsvUtils::readTable(stadium_csv, csv, csv_error))
    {
        _last_error = csv_error;
        return false;
    }

    const QHash<QString, int> header_index = CsvUtils::buildHeaderIndex(csv.header);
    const int team_idx = CsvUtils::findHeaderIndex(header_index, { "team_name", "team" });
    const int stadium_idx = CsvUtils::findHeaderIndex(header_index, { "stadium_name", "stadium" });
    const int capacity_idx = CsvUtils::findHeaderIndex(header_index, { "seating_capacity" });
    const int location_idx = CsvUtils::findHeaderIndex(header_index, { "location" });
    const int surface_idx = CsvUtils::findHeaderIndex(header_index, { "playing_surface" });
    const int league_idx = CsvUtils::findHeaderIndex(header_index, { "league" });
    const int date_opened_idx = CsvUtils::findHeaderIndex(header_index, { "date_opened" });
    const int center_field_idx = CsvUtils::findHeaderIndex(header_index, { "distance_to_center_field" });
    const int typology_idx = CsvUtils::findHeaderIndex(header_index, { "ballpark_typology" });
    const int roof_type_idx = CsvUtils::findHeaderIndex(header_index, { "roof_type" });

    if (team_idx < 0 || stadium_idx < 0)
    {
        _last_error = "stadium csv header missing required columns in file: " + stadium_csv;
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(_conn_name);
    if (!db.isValid() || !db.isOpen())
    {
        _last_error = "Database is not open.";
        return false;
    }

    if (!db.transaction())
    {
        _last_error = "begin stadium import transaction failed: " + db.lastError().text();
        return false;
    }

    QSqlQuery insert_q(db);
    insert_q.prepare(R"SQL(
        INSERT INTO stadiums
        (
            team_name,
            stadium_name,
            seating_capacity,
            location,
            playing_surface,
            league,
            date_opened,
            distance_to_center_field_ft,
            distance_to_center_field_raw,
            ballpark_typology,
            roof_type,
            is_expansion
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(team_name) DO UPDATE SET
            stadium_name = excluded.stadium_name,
            seating_capacity = excluded.seating_capacity,
            location = excluded.location,
            playing_surface = excluded.playing_surface,
            league = excluded.league,
            date_opened = excluded.date_opened,
            distance_to_center_field_ft = excluded.distance_to_center_field_ft,
            distance_to_center_field_raw = excluded.distance_to_center_field_raw,
            ballpark_typology = excluded.ballpark_typology,
            roof_type = excluded.roof_type,
            is_expansion = excluded.is_expansion
        ON CONFLICT(stadium_name) DO UPDATE SET
            team_name = excluded.team_name,
            seating_capacity = excluded.seating_capacity,
            location = excluded.location,
            playing_surface = excluded.playing_surface,
            league = excluded.league,
            date_opened = excluded.date_opened,
            distance_to_center_field_ft = excluded.distance_to_center_field_ft,
            distance_to_center_field_raw = excluded.distance_to_center_field_raw,
            ballpark_typology = excluded.ballpark_typology,
            roof_type = excluded.roof_type,
            is_expansion = excluded.is_expansion;
    )SQL");

    const int is_expansion = stadium_csv.contains("Expansion", Qt::CaseInsensitive) ? 1 : 0;

    int processed_rows = 0;
    int skipped_rows = 0;
    int imported_rows = 0;

    for (const QStringList& row : csv.rows)
    {
        ++processed_rows;

        const QString team_name = CsvUtils::cellAt(row, team_idx).trimmed();
        const QString stadium_name = CsvUtils::cellAt(row, stadium_idx).trimmed();

        if (team_name.isEmpty() || stadium_name.isEmpty())
        {
            ++skipped_rows;
            continue;
        }

        const QString capacity_txt = CsvUtils::cellAt(row, capacity_idx);
        const QString location = CsvUtils::cellAt(row, location_idx).trimmed();
        const QString playing_surface = CsvUtils::cellAt(row, surface_idx).trimmed();
        const QString league = CsvUtils::cellAt(row, league_idx).trimmed();
        const QString date_opened_txt = CsvUtils::cellAt(row, date_opened_idx);
        const QString center_field_raw = CsvUtils::cellAt(row, center_field_idx).trimmed();
        const QString typology = CsvUtils::cellAt(row, typology_idx).trimmed();
        const QString roof_type = CsvUtils::cellAt(row, roof_type_idx).trimmed();

        insert_q.bindValue(0, team_name);
        insert_q.bindValue(1, stadium_name);
        insert_q.bindValue(2, CsvUtils::parseIntLoose(capacity_txt, 0));
        insert_q.bindValue(3, location);
        insert_q.bindValue(4, playing_surface);
        insert_q.bindValue(5, league);
        insert_q.bindValue(6, CsvUtils::parseIntLoose(date_opened_txt, 0));
        insert_q.bindValue(7, CsvUtils::parseFirstNumber(center_field_raw, 0.0));
        insert_q.bindValue(8, center_field_raw);
        insert_q.bindValue(9, typology);
        insert_q.bindValue(10, roof_type);
        insert_q.bindValue(11, is_expansion);

        if (!insert_q.exec())
        {
            _last_error =
                "import stadium csv failed at row [" + QString::number(processed_rows) +
                "] [" + team_name + " / " + stadium_name + "]: " +
                insert_q.lastError().text();
            db.rollback();
            return false;
        }

        ++imported_rows;
    }

    if (imported_rows == 0)
    {
        _last_error =
            "stadium csv import produced no valid rows. processed=" +
            QString::number(processed_rows) +
            ", skipped=" + QString::number(skipped_rows);
        db.rollback();
        return false;
    }

    if (!db.commit())
    {
        _last_error = "commit stadium import failed: " + db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}

bool DatabaseManager::import_distances_csv_file(const QString& distances_csv)
{
    _last_error.clear();
    _last_warning.clear();

    CsvUtils::Table csv;
    QString csv_error;
    if (!CsvUtils::readTable(distances_csv, csv, csv_error))
    {
        _last_error = "read distance csv failed: " + csv_error;
        return false;
    }

    const QHash<QString, int> header_index = CsvUtils::buildHeaderIndex(csv.header);
    const int from_idx = CsvUtils::findHeaderIndex(
        header_index,
        { "originated_stadtium", "originated_stadium", "h1", "from_stadium", "from" });

    const int to_idx = CsvUtils::findHeaderIndex(
        header_index,
        { "destination_stadtium", "destination_stadium", "team", "to_stadium", "to" });

    const int distance_idx = CsvUtils::findHeaderIndex(
        header_index,
        { "distance", "mileage", "miles" });

    if (from_idx < 0 || to_idx < 0 || distance_idx < 0)
    {
        _last_error = "distance csv header missing required columns in file: " + distances_csv;
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(_conn_name);
    if (!db.isValid() || !db.isOpen())
    {
        _last_error = "Database is not open.";
        return false;
    }

    QHash<QString, int> stadium_id_by_name;
    {
        QSqlQuery stadium_q(db);
        if (!stadium_q.exec("SELECT stadium_id, stadium_name FROM stadiums;"))
        {
            _last_error = "load stadium ids for distance import failed: " + stadium_q.lastError().text();
            return false;
        }

        while (stadium_q.next())
        {
            const int stadium_id = stadium_q.value(0).toInt();
            const QString stadium_name = stadium_q.value(1).toString();
            stadium_id_by_name.insert(CsvUtils::normalizeStadiumNameKey(stadium_name), stadium_id);
        }
    }

    if (stadium_id_by_name.isEmpty())
    {
        _last_error = "distance import failed: no stadium records available.";
        return false;
    }

    if (!db.transaction())
    {
        _last_error = "begin distance import transaction failed: " + db.lastError().text();
        return false;
    }

    QSqlQuery insert_q(db);
    insert_q.prepare(R"SQL(
        INSERT INTO stadium_distances(stadium_a_id, stadium_b_id, distance_miles)
        VALUES (?, ?, ?)
        ON CONFLICT(stadium_a_id, stadium_b_id) DO UPDATE SET
            distance_miles = excluded.distance_miles;
    )SQL");

    int processed_rows = 0;
    int skipped_empty_rows = 0;
    int skipped_unknown_stadium_rows = 0;
    int skipped_invalid_distance_rows = 0;
    int skipped_self_loop_rows = 0;
    int imported_rows = 0;

    for (const QStringList& row : csv.rows)
    {
        ++processed_rows;

        const QString from_name = CsvUtils::cellAt(row, from_idx).trimmed();
        const QString to_name = CsvUtils::cellAt(row, to_idx).trimmed();
        const QString distance_txt = CsvUtils::cellAt(row, distance_idx).trimmed();

        if (from_name.isEmpty() || to_name.isEmpty())
        {
            ++skipped_empty_rows;
            continue;
        }

        const QString from_key = CsvUtils::normalizeStadiumNameKey(from_name);
        const QString to_key = CsvUtils::normalizeStadiumNameKey(to_name);

        if (!stadium_id_by_name.contains(from_key) || !stadium_id_by_name.contains(to_key))
        {
            ++skipped_unknown_stadium_rows;
            continue;
        }

        const int from_id = stadium_id_by_name.value(from_key);
        const int to_id = stadium_id_by_name.value(to_key);

        if (from_id == to_id)
        {
            ++skipped_self_loop_rows;
            continue;
        }

        const double distance_miles = CsvUtils::parseFirstNumber(distance_txt, -1.0);
        if (distance_miles < 0.0)
        {
            ++skipped_invalid_distance_rows;
            continue;
        }

        const int stadium_a_id = std::min(from_id, to_id);
        const int stadium_b_id = std::max(from_id, to_id);

        insert_q.bindValue(0, stadium_a_id);
        insert_q.bindValue(1, stadium_b_id);
        insert_q.bindValue(2, distance_miles);

        if (!insert_q.exec())
        {
            _last_error =
                "import distance csv failed at row [" + QString::number(processed_rows) +
                "] [" + from_name + " -> " + to_name + "]: " +
                insert_q.lastError().text();
            db.rollback();
            return false;
        }

        ++imported_rows;
    }

    if (imported_rows == 0)
    {
        _last_error =
            "distance csv import produced no valid rows. processed=" + QString::number(processed_rows) +
            ", skipped_empty=" + QString::number(skipped_empty_rows) +
            ", skipped_unknown_stadium=" + QString::number(skipped_unknown_stadium_rows) +
            ", skipped_invalid_distance=" + QString::number(skipped_invalid_distance_rows) +
            ", skipped_self_loop=" + QString::number(skipped_self_loop_rows);
        db.rollback();
        return false;
    }

    if (!db.commit())
    {
        _last_error = "commit distance import failed: " + db.lastError().text();
        db.rollback();
        return false;
    }

    QStringList warnings;
    if (skipped_empty_rows > 0)
        MessageUtils::appendUniqueMessage(warnings, "distance csv skipped empty rows: " + QString::number(skipped_empty_rows));
    if (skipped_unknown_stadium_rows > 0)
        MessageUtils::appendUniqueMessage(warnings, "distance csv skipped unknown stadium rows: " + QString::number(skipped_unknown_stadium_rows));
    if (skipped_invalid_distance_rows > 0)
        MessageUtils::appendUniqueMessage(warnings, "distance csv skipped invalid distance rows: " + QString::number(skipped_invalid_distance_rows));
    if (skipped_self_loop_rows > 0)
        MessageUtils::appendUniqueMessage(warnings, "distance csv skipped self-loop rows: " + QString::number(skipped_self_loop_rows));

    _last_warning = MessageUtils::joinMessages(warnings);

    return true;
}

DatabaseManager::SchemaCheckResult DatabaseManager::check_table_schema_compatible()
{
    QSqlDatabase db = QSqlDatabase::database(_conn_name, false);
    if (!db.isValid() || !db.isOpen())
    {
        _last_error = "schema compatibility check failed: database is not open.";
        return SchemaCheckResult::CheckFailed;
    }

    auto table_exists = [&](const QString& table_name, bool& exists_out) -> SchemaCheckResult
        {
            QSqlQuery q(db);
            q.prepare("SELECT 1 FROM sqlite_master WHERE type='table' AND name=? LIMIT 1;");
            q.bindValue(0, table_name);

            if (!q.exec())
            {
                _last_error =
                    "schema compatibility check failed while checking table ["
                    + table_name + "]: " + q.lastError().text();
                return SchemaCheckResult::CheckFailed;
            }

            exists_out = q.next();
            return SchemaCheckResult::Compatible;
        };

    auto table_contains_required_columns =
        [&](const QString& table_name, const QSet<QString>& required_columns) -> SchemaCheckResult
        {
            QSqlQuery q(db);
            if (!q.exec("PRAGMA table_info(" + table_name + ");"))
            {
                _last_error =
                    "schema compatibility check failed while reading table info ["
                    + table_name + "]: " + q.lastError().text();
                return SchemaCheckResult::CheckFailed;
            }

            QSet<QString> existing_columns;
            while (q.next())
                existing_columns.insert(q.value(1).toString().trimmed().toLower());

            if (existing_columns.isEmpty())
            {
                _last_error = "schema compatibility check failed: table [" + table_name + "] has no columns.";
                return SchemaCheckResult::Incompatible;
            }

            for (const QString& required : required_columns)
            {
                if (!existing_columns.contains(required))
                {
                    _last_error =
                        "schema mismatch on table [" + table_name + "], missing column ["
                        + required + "]";
                    return SchemaCheckResult::Incompatible;
                }
            }

            return SchemaCheckResult::Compatible;
        };

    auto validate_table_if_exists =
        [&](const QString& table_name, const QSet<QString>& required_columns) -> SchemaCheckResult
        {
            bool exists = false;
            const SchemaCheckResult table_exists_result = table_exists(table_name, exists);
            if (table_exists_result != SchemaCheckResult::Compatible)
                return table_exists_result;

            if (!exists)
                return SchemaCheckResult::Compatible;

            return table_contains_required_columns(table_name, required_columns);
        };

    const SchemaCheckResult stadiums_check_result = validate_table_if_exists("stadiums",
        {
            "stadium_id",
            "team_name",
            "stadium_name",
            "seating_capacity",
            "location",
            "playing_surface",
            "league",
            "date_opened",
            "distance_to_center_field_ft",
            "distance_to_center_field_raw",
            "ballpark_typology",
            "roof_type",
            "is_expansion"
        });
    if (stadiums_check_result != SchemaCheckResult::Compatible)
        return stadiums_check_result;

    const SchemaCheckResult souvenirs_check_result = validate_table_if_exists("souvenirs",
        {
            "souvenir_id",
            "stadium_id",
            "name",
            "price"
        });
    if (souvenirs_check_result != SchemaCheckResult::Compatible)
        return souvenirs_check_result;

    const SchemaCheckResult distances_check_result = validate_table_if_exists("stadium_distances",
        {
            "stadium_a_id",
            "stadium_b_id",
            "distance_miles"
        });
    if (distances_check_result != SchemaCheckResult::Compatible)
        return distances_check_result;

    _last_error.clear();
    return SchemaCheckResult::Compatible;
}
