#include "stadium_repository.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

StadiumRepository::StadiumRepository(DatabaseManager& db_manager)
	: _db_manager(db_manager) {}


std::vector<Stadium> StadiumRepository::getAllStadiums(StadiumSortBy sort_by, LeagueFilter league) const {
	std::vector<Stadium> tmp;

	const QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return tmp;

    QString sql = R"(
        SELECT
            stadium_id,
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
        FROM stadiums
    )";

    QString sort;
    switch (sort_by)
    {
    case StadiumSortBy::TeamName:
        sort = "team_name ASC, stadium_name ASC";
        break;
    case StadiumSortBy::StadiumName:
        sort = "stadium_name ASC, team_name ASC";
        break;
    case StadiumSortBy::DateOpened:
        sort = "date_opened ASC, stadium_name ASC";
        break;
    case StadiumSortBy::SeatingCapacity:
        sort = "seating_capacity ASC, stadium_name ASC";
        break;
    case StadiumSortBy::Typology:
        sort = "ballpark_typology ASC, stadium_name ASC";
        break;
    default:
        return tmp;
    }
    
    switch (league)
    {
    case LeagueFilter::All:
        break;
    case LeagueFilter::American:
        sql += " WHERE league = 'American' ";
        break;
    case LeagueFilter::National:
        sql += " WHERE league = 'National' ";
        break;
    default:
        return tmp;
    }
  
    sql += " ORDER BY " + sort;

    QSqlQuery q(db);
    if (!q.prepare(sql))
        return tmp;

    if (!q.exec())
        return tmp;

    while (q.next())
    {
        tmp.push_back(buildStadiumFromQuery(q));
    }

	return tmp;
}

std::optional<Stadium> StadiumRepository::getStadiumByID(int stadium_id)
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return std::nullopt;

	QSqlQuery q(db);
	q.prepare(R"SQL(
        SELECT
            stadium_id,
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
        FROM stadiums
        WHERE stadium_id = ?
        LIMIT 1;
    )SQL");

	q.addBindValue(stadium_id);

	if (!q.exec())
		return std::nullopt;

	if (!q.next())
		return std::nullopt;

	return buildStadiumFromQuery(q);
}

std::optional<Stadium> StadiumRepository::getStadiumByStadiumName(const QString& stadium_name)
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return std::nullopt;

	QSqlQuery q(db);
	q.prepare(R"SQL(
        SELECT
            stadium_id,
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
        FROM stadiums
        WHERE stadium_name = ?
        LIMIT 1;
    )SQL");

	q.addBindValue(stadium_name);

	if (!q.exec())
		return std::nullopt;

	if (!q.next())
		return std::nullopt;

	return buildStadiumFromQuery(q);
}

std::optional<Stadium> StadiumRepository::getStadiumByTeamName(const QString& team_name)
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return std::nullopt;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
        SELECT
            stadium_id,
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
        FROM stadiums
        WHERE team_name = ?
        LIMIT 1;
    )SQL"))
		return std::nullopt;

	q.addBindValue(team_name);

	if (!q.exec())
		return std::nullopt;

	if (!q.next())
		return std::nullopt;

	return buildStadiumFromQuery(q);

}

std::optional<int> StadiumRepository::getStadiumID(const QString& stadium_name)
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return std::nullopt;

	QSqlQuery q(db);
	q.prepare(R"SQL(
        SELECT stadium_id
        FROM stadiums
        WHERE stadium_name = ?
        LIMIT 1;
    )SQL");

	q.addBindValue(stadium_name);

	if (!q.exec())
		return std::nullopt;

	if (!q.next())
		return std::nullopt;

	return q.value("stadium_id").toInt();
	//return q.value("0").toInt();

}

std::optional<QString> StadiumRepository::getStadiumName(int stadium_id)
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return std::nullopt;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
        SELECT stadium_name
        FROM stadiums
        WHERE stadium_id = ?
        LIMIT 1;
    )SQL"))
		return std::nullopt;

	q.addBindValue(stadium_id);

	if (!q.exec())
		return std::nullopt;

	if (!q.next())
		return std::nullopt;

	return q.value("stadium_name").toString();

}

Stadium StadiumRepository::buildStadiumFromQuery(const QSqlQuery& q) const
{
	Stadium stadium;

	stadium.stadium_id = q.value("stadium_id").toInt();
	stadium.team_name = q.value("team_name").toString();
	stadium.stadium_name = q.value("stadium_name").toString();
	stadium.seating_capacity = q.value("seating_capacity").toInt();
	stadium.location = q.value("location").toString();
	stadium.playing_surface = q.value("playing_surface").toString();
	stadium.league = q.value("league").toString();
	stadium.date_opened = q.value("date_opened").toInt();
	stadium.distance_to_center_field_ft = q.value("distance_to_center_field_ft").toInt();
	stadium.distance_to_center_field_raw = q.value("distance_to_center_field_raw").toString();
	stadium.ballpark_typology = q.value("ballpark_typology").toString();
	stadium.roof_type = q.value("roof_type").toString();
	stadium.is_expansion = q.value("is_expansion").toBool();

	return stadium;
}

bool StadiumRepository::addStadium(const Stadium& new_staduim)
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return false;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
        INSERT INTO stadiums
        (
            stadium_id,
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
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )SQL"))
		return false;

	if (new_staduim.stadium_id > 0)
		q.addBindValue(new_staduim.stadium_id);
	else
		q.addBindValue(QVariant());
	q.addBindValue(new_staduim.team_name);
	q.addBindValue(new_staduim.stadium_name);
	q.addBindValue(new_staduim.seating_capacity);
	q.addBindValue(new_staduim.location);
	q.addBindValue(new_staduim.playing_surface);
	q.addBindValue(new_staduim.league);
	q.addBindValue(new_staduim.date_opened);
	q.addBindValue(new_staduim.distance_to_center_field_ft);
	q.addBindValue(new_staduim.distance_to_center_field_raw);
	q.addBindValue(new_staduim.ballpark_typology);
	q.addBindValue(new_staduim.roof_type);
	q.addBindValue(new_staduim.is_expansion ? 1 : 0);

	return q.exec();
}

bool StadiumRepository::upDateStadiumInform(int stadium_id, const Stadium& update_staduim)
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return false;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
        UPDATE stadiums
        SET
            team_name = ?,
            stadium_name = ?,
            seating_capacity = ?,
            location = ?,
            playing_surface = ?,
            league = ?,
            date_opened = ?,
            distance_to_center_field_ft = ?,
            distance_to_center_field_raw = ?,
            ballpark_typology = ?,
            roof_type = ?,
            is_expansion = ?
        WHERE stadium_id = ?;
    )SQL"))
		return false;

	q.addBindValue(update_staduim.team_name);
	q.addBindValue(update_staduim.stadium_name);
	q.addBindValue(update_staduim.seating_capacity);
	q.addBindValue(update_staduim.location);
	q.addBindValue(update_staduim.playing_surface);
	q.addBindValue(update_staduim.league);
	q.addBindValue(update_staduim.date_opened);
	q.addBindValue(update_staduim.distance_to_center_field_ft);
	q.addBindValue(update_staduim.distance_to_center_field_raw);
	q.addBindValue(update_staduim.ballpark_typology);
	q.addBindValue(update_staduim.roof_type);
	q.addBindValue(update_staduim.is_expansion ? 1 : 0);
	q.addBindValue(stadium_id);

	if (!q.exec())
		return false;

	return q.numRowsAffected() > 0 || getStadiumByID(stadium_id).has_value();
}

bool StadiumRepository::deleteStadium(int stadium_id)
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return false;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
        DELETE FROM stadiums
        WHERE stadium_id = ?;
    )SQL"))
		return false;

	q.addBindValue(stadium_id);

	if (!q.exec())
		return false;

	return q.numRowsAffected() > 0;
}

bool StadiumRepository::stadiumNameExists(const QString& stadium_name) const
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return false;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
        SELECT 1
        FROM stadiums
        WHERE stadium_name = ?
        LIMIT 1;
    )SQL"))
		return false;

	q.addBindValue(stadium_name);

	if (!q.exec())
		return false;

	return q.next();
}

bool StadiumRepository::teamNameExists(const QString& team_name) const
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return false;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
        SELECT 1
        FROM stadiums
        WHERE team_name = ?
        LIMIT 1;
    )SQL"))
		return false;

	q.addBindValue(team_name);

	if (!q.exec())
		return false;

	return q.next();
}
