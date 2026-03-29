#include "database_manager.h"
#include "csv_utils.h"

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

}

//-------------------------------------public--------------------------------------

DatabaseManager::DatabaseManager(){}

bool DatabaseManager::init()
{
    return init_impl(true);
}

bool DatabaseManager::resetDatabase(bool remove_backup_if_success)
{
    _last_error.clear();

    _initialize_all = false;

    if (QSqlDatabase::contains(_conn_name))
    {
        {
            QSqlDatabase db = QSqlDatabase::database(_conn_name, false);
            if (db.isValid() && db.isOpen())
                db.close();
        }
        QSqlDatabase::removeDatabase(_conn_name);

    }

    QString time = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");

    if (QFile::exists(_db_path))
    {
        QString backup = _db_path + time + ".bak";
        if (!QFile::rename(_db_path, backup))
        {
            _last_error = "failed to backup db file";
            _initialize_all = false;
            if (init_impl(false))
                _last_error += ", but database is still usable";
            else
                _last_error += ", and database is not usable";

            return false;
        }
    }


    if (!init_impl(false))
    {
        QFile::remove(_db_path);
        QFile::rename(_db_path + time + ".bak", _db_path);

        _initialize_all = false;
        _last_error = "failed to initialize new database";
        if (init_impl(false))
            _last_error += ", but restored backup successfully";
        else
            _last_error += ", and failed to restore backup";

        return false;
    }

    if (remove_backup_if_success)
        QFile::remove(_db_path + time + ".bak");

    return true;
}

bool DatabaseManager::isOpen() const
{
    if (!QSqlDatabase::contains(_conn_name))
        return false;

    const QSqlDatabase db = QSqlDatabase::database(_conn_name, false);

    return db.isValid() && db.isOpen();
}

bool DatabaseManager::isStadiumsAvailable() const
{
    return _init_stadiums;
}

bool DatabaseManager::isSouvenirsAvailable() const
{
    return _init_souvenirs;
}

bool DatabaseManager::isDistanceAvailable() const
{
    return _init_distances;
}

const QString DatabaseManager::lastError() const
{
	return _last_error;
}

QSqlDatabase DatabaseManager::getDatabaseObj() const
{
    if (!QSqlDatabase::contains(_conn_name))
        return QSqlDatabase();

    return QSqlDatabase::database(_conn_name, false);
}

//-------------------------------------private--------------------------------------

bool DatabaseManager::init_impl(bool allow_auto_recover)
{
    if (_initialize_all)
        return true;

    _last_error.clear();

    if (!open_db())
        return false;

    if (!init_pragmas())
        return false;

    if (!init_schema())
        return false;

    if (!validate_schema_compatibility())
    {
        const QString schema_error = _last_error;
        if (!allow_auto_recover)
            return false;

        if (!resetDatabase(false))
        {
            const QString reload_error = _last_error;
            _last_error = "Schema mismatch detected: " + schema_error
                + " | Auto-reload failed: " + reload_error;
            return false;
        }

        _last_error = "Schema mismatch detected and auto-reload completed: " + schema_error;
        return true;
    }

    if (!seed_if_empty())
        return false;

    _initialize_all = true;

    return true;
}

bool DatabaseManager::open_db()
{
    std::cout << "1-Start.Opening the database" << std::endl;
    _last_error.clear();

    QDir dir(QDir::current());

    QString data_dir_path;
    for (int i = 0; i < 8; ++i)
    {
        const QString candidate = dir.filePath("data");
        if (QDir(candidate).exists())
        {
            data_dir_path = QDir(candidate).absolutePath();
            break;
        }

        const QString candidate2 = dir.filePath("App/data");
        if (QDir(candidate2).exists())
        {
            data_dir_path = QDir(candidate2).absolutePath();
            break;
        }

        if (!dir.cdUp())
            break;
    }

    if (data_dir_path.isEmpty())
    {
        const QString fallback = QDir::current().filePath("data");
        QDir().mkpath(fallback);
        data_dir_path = QDir(fallback).absolutePath();
    }

    _db_path = QDir(data_dir_path).filePath("BaseBall_data.db");

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
    std::cout << "1-Finish.Opening the database" << std::endl;

    return true;
}

bool DatabaseManager::load_table_columns(QSqlDatabase& db, const QString& table_name, QSet<QString>& out_columns, QString& out_error) const
{
    out_columns.clear();

    QSqlQuery q(db);
    if (!q.exec("PRAGMA table_info(" + table_name + ");"))
    {
        out_error = q.lastError().text() + " | SQL: PRAGMA table_info(" + table_name + ")";
        return false;
    }

    while (q.next())
    {
        const QString column_name = q.value("name").toString().trimmed().toLower();
        if (!column_name.isEmpty())
            out_columns.insert(column_name);
    }

    if (out_columns.isEmpty())
    {
        out_error = "Table is missing or has no columns: " + table_name;
        return false;
    }

    return true;
}

bool DatabaseManager::upsert_default_souvenirs_for_all_stadiums(QSqlDatabase& db, QString& out_error)
{
    QSqlQuery stadium_q(db);
    if (!stadium_q.exec("SELECT stadium_id FROM stadiums;"))
    {
        out_error = stadium_q.lastError().text() + " | SQL: SELECT stadium_id FROM stadiums";
        return false;
    }

    QVector<int> stadium_ids;
    while (stadium_q.next())
        stadium_ids.push_back(stadium_q.value(0).toInt());

    if (stadium_ids.isEmpty())
        return true;

    if (!db.transaction())
    {
        out_error = db.lastError().text();
        return false;
    }

    QSqlQuery upsert_q(db);
    upsert_q.prepare(R"SQL(
        INSERT INTO souvenirs(stadium_id, name, price)
        VALUES (?, ?, ?)
        ON CONFLICT(stadium_id, name) DO UPDATE SET
            price = excluded.price;
    )SQL");

    for (const int stadium_id : stadium_ids)
    {
        for (const DefaultSouvenir& item : k_default_souvenirs)
        {
            upsert_q.bindValue(0, stadium_id);
            upsert_q.bindValue(1, QString::fromUtf8(item.name));
            upsert_q.bindValue(2, item.price);
            if (!upsert_q.exec())
            {
                out_error = upsert_q.lastError().text();
                db.rollback();
                return false;
            }
        }
    }

    if (!db.commit())
    {
        out_error = db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}

bool DatabaseManager::init_pragmas()
{
    std::cout << "2-Start Configuring runtime parameters" << std::endl;

    _last_error.clear();

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

    std::cout << "2-Finish Configuring runtime parameters" << std::endl;

    return true;
}

bool DatabaseManager::init_schema()
{
    std::cout << "3-Start Building the table structure" << std::endl;
    _last_error.clear();

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
        return false;
    }

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
        return false;
    }

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
        return false;
    }

    std::cout << "3-Finish Building the table structure" << std::endl;

    return true;
}

bool DatabaseManager::validate_schema_compatibility()
{
    _last_error.clear();

    QSqlDatabase db = QSqlDatabase::database(_conn_name);
    if (!db.isValid() || !db.isOpen())
    {
        _last_error = "Database is not open.";
        return false;
    }

    const QHash<QString, QVector<QString>> required_columns =
    {
        {
            "stadiums",
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
            }
        },
        {
            "souvenirs",
            {
                "souvenir_id",
                "stadium_id",
                "name",
                "price"
            }
        },
        {
            "stadium_distances",
            {
                "stadium_a_id",
                "stadium_b_id",
                "distance_miles"
            }
        }
    };

    for (auto it = required_columns.constBegin(); it != required_columns.constEnd(); ++it)
    {
        const QString& table_name = it.key();
        const QVector<QString>& columns = it.value();

        QSet<QString> actual_columns;
        QString schema_read_error;
        if (!load_table_columns(db, table_name, actual_columns, schema_read_error))
        {
            _last_error = "[schema-check] " + schema_read_error;
            return false;
        }

        QStringList missing_columns;
        for (const QString& expected_col : columns)
        {
            if (!actual_columns.contains(expected_col))
                missing_columns.push_back(expected_col);
        }

        if (!missing_columns.isEmpty())
        {
            _last_error = "[schema-check] Table '" + table_name
                + "' missing columns: " + missing_columns.join(", ");
            return false;
        }
    }

    return true;
}

bool DatabaseManager::seed_if_empty()
{
    std::cout << "4-Start Loading data" << std::endl;
    _last_error.clear();

    QSqlDatabase db = QSqlDatabase::database(_conn_name);
    if (!db.isValid() || !db.isOpen())
    {
        _last_error = "Database is not open.";
        return false;
    }

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

    const int stadium_count = table_row_count("stadiums");
    if (stadium_count < 0)
        return false;

    const int distance_count = table_row_count("stadium_distances");
    if (distance_count < 0)
        return false;

    const QString data_dir = QFileInfo(_db_path).absolutePath();
    const QString stadium_csv = QDir(data_dir).filePath("MLB Information.csv");
    const QString distances_csv = QDir(data_dir).filePath("Distance between stadiums.csv");

    if (stadium_count == 0)
    {
        if (!import_stadium_from_csv_files(stadium_csv))
            return false;
    }

    if (!upsert_default_souvenirs_for_all_stadiums(db, _last_error))
        return false;

    if (distance_count == 0)
    {
        if (!import_distances_csv_file(distances_csv))
            return false;
    }

    std::cout << "4-Finish Loading data" << std::endl;

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
        _last_error = "CSV header is missing required columns in file: " + stadium_csv;
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
        _last_error = db.lastError().text();
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

    for (const QStringList& row : csv.rows)
    {
        const QString team_name = CsvUtils::cellAt(row, team_idx);
        const QString stadium_name = CsvUtils::cellAt(row, stadium_idx);
        if (team_name.isEmpty() || stadium_name.isEmpty())
            continue;

        const QString capacity_txt = CsvUtils::cellAt(row, capacity_idx);
        const QString location = CsvUtils::cellAt(row, location_idx);
        const QString playing_surface = CsvUtils::cellAt(row, surface_idx);
        const QString league = CsvUtils::cellAt(row, league_idx);
        const QString date_opened_txt = CsvUtils::cellAt(row, date_opened_idx);
        const QString center_field_raw = CsvUtils::cellAt(row, center_field_idx);
        const QString typology = CsvUtils::cellAt(row, typology_idx);
        const QString roof_type = CsvUtils::cellAt(row, roof_type_idx);

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
            _last_error = insert_q.lastError().text();
            db.rollback();
            return false;
        }
    }

    if (!db.commit())
    {
        _last_error = db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}

bool DatabaseManager::import_distances_csv_file(const QString& distances_csv)
{
    _last_error.clear();

    CsvUtils::Table csv;
    QString csv_error;
    if (!CsvUtils::readTable(distances_csv, csv, csv_error))
    {
        _last_error = csv_error;
        return false;
    }

    const QHash<QString, int> header_index = CsvUtils::buildHeaderIndex(csv.header);
    const int from_idx = CsvUtils::findHeaderIndex(header_index, { "originated_stadtium", "originated_stadium", "h1", "from_stadium", "from" });
    const int to_idx = CsvUtils::findHeaderIndex(header_index, { "destination_stadtium", "destination_stadium", "team", "to_stadium", "to" });
    const int distance_idx = CsvUtils::findHeaderIndex(header_index, { "distance", "mileage", "miles" });

    if (from_idx < 0 || to_idx < 0 || distance_idx < 0)
    {
        _last_error = "CSV header is missing required columns in file: " + distances_csv;
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
            _last_error = stadium_q.lastError().text();
            return false;
        }

        while (stadium_q.next())
        {
            const int stadium_id = stadium_q.value(0).toInt();
            const QString stadium_name = stadium_q.value(1).toString();
            stadium_id_by_name.insert(CsvUtils::normalizeStadiumNameKey(stadium_name), stadium_id);
        }
    }

    if (!db.transaction())
    {
        _last_error = db.lastError().text();
        return false;
    }

    QSqlQuery insert_q(db);
    insert_q.prepare(R"SQL(
        INSERT INTO stadium_distances(stadium_a_id, stadium_b_id, distance_miles)
        VALUES (?, ?, ?)
        ON CONFLICT(stadium_a_id, stadium_b_id) DO UPDATE SET
            distance_miles = excluded.distance_miles;
    )SQL");

    for (const QStringList& row : csv.rows)
    {
        const QString from_name = CsvUtils::cellAt(row, from_idx);
        const QString to_name = CsvUtils::cellAt(row, to_idx);
        const QString distance_txt = CsvUtils::cellAt(row, distance_idx);

        if (from_name.isEmpty() || to_name.isEmpty())
            continue;

        const QString from_key = CsvUtils::normalizeStadiumNameKey(from_name);
        const QString to_key = CsvUtils::normalizeStadiumNameKey(to_name);

        if (!stadium_id_by_name.contains(from_key) || !stadium_id_by_name.contains(to_key))
        {
            _last_error = "Distance csv references unknown stadium: " + from_name + " -> " + to_name;
            db.rollback();
            return false;
        }

        const int from_id = stadium_id_by_name.value(from_key);
        const int to_id = stadium_id_by_name.value(to_key);
        if (from_id == to_id)
            continue;

        const int stadium_a_id = std::min(from_id, to_id);
        const int stadium_b_id = std::max(from_id, to_id);
        const double distance_miles = CsvUtils::parseFirstNumber(distance_txt, -1.0);
        if (distance_miles < 0.0)
            continue;

        insert_q.bindValue(0, stadium_a_id);
        insert_q.bindValue(1, stadium_b_id);
        insert_q.bindValue(2, distance_miles);
        if (!insert_q.exec())
        {
            _last_error = insert_q.lastError().text();
            db.rollback();
            return false;
        }
    }

    if (!db.commit())
    {
        _last_error = db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}
