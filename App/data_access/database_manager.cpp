#include "database_manager.h"

DatabaseManager::DatabaseManager(){}

bool DatabaseManager::init()
{
    if (_initialize)
        return true;

    if (!open_db())
        return false;

    if (!init_pragmas())
        return false;

    if (!init_schema())
        return false;

    if (!seed_if_empty())
        return false;

    _initialize = true;

    return true;

}

bool DatabaseManager::resetDatabase(bool remove_backup_if_success)
{

}

bool DatabaseManager::isOpen() const
{
    if (!QSqlDatabase::contains(_conn_name))
        return false;

    const QSqlDatabase db = QSqlDatabase::database(_conn_name, false);

    return db.isValid() && db.isOpen();
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

    db.setDatabaseName(_db_path);

    if (!db.open())
    {
        _last_error = db.lastError().text();
        return false;
    }

    std::cout <<" "<<_db_path.toStdString() << std::endl;
    std::cout << "1-Finish.Opening the database" << std::endl;

    return true;
}

bool DatabaseManager::init_pragmas()
{
    std::cout << "2-Start Configuring runtime parameters" << std::endl;

    _last_error.clear();

    QSqlDatabase db = QSqlDatabase::database(_conn_name);
    if (db.isValid() && db.isOpen())
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

    db.setConnectOptions("QSQLITE_BUSY_TIMEOUT=5000");

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

    if (!db.transaction())
    {
        _last_error = db.lastError().text();
        return false;
    }

    // Tables are designed from assets/*.csv:
    // 1) MLB Information*.csv -> stadiums
    // 2) Distance between*.csv -> stadium_distances
    const QVector<QString> schema_sql =
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
        R"SQL(
            CREATE TABLE IF NOT EXISTS stadium_distances
            (
                from_stadium_id INTEGER NOT NULL,
                to_stadium_id INTEGER NOT NULL,
                distance_miles REAL NOT NULL CHECK (distance_miles >= 0),
                PRIMARY KEY(from_stadium_id, to_stadium_id),
                CHECK (from_stadium_id <> to_stadium_id),
                FOREIGN KEY(from_stadium_id) REFERENCES stadiums(stadium_id)
                    ON UPDATE CASCADE
                    ON DELETE CASCADE,
                FOREIGN KEY(to_stadium_id) REFERENCES stadiums(stadium_id)
                    ON UPDATE CASCADE
                    ON DELETE CASCADE
            );
        )SQL",
        "CREATE INDEX IF NOT EXISTS idx_stadiums_league ON stadiums(league);",
        "CREATE INDEX IF NOT EXISTS idx_stadiums_name ON stadiums(stadium_name);",
        "CREATE INDEX IF NOT EXISTS idx_distances_from ON stadium_distances(from_stadium_id);",
        "CREATE INDEX IF NOT EXISTS idx_distances_to ON stadium_distances(to_stadium_id);"
    };

    for (const QString& sql : schema_sql)
    {
        if (!exec_sql(sql))
        {
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

    std::cout << "3-Finish Building the table structure" << std::endl;

    return true;
}

bool DatabaseManager::seed_if_empty()
{

    return true;
}


