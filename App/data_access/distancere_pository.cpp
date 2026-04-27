#include "distancere_pository.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <algorithm>

namespace
{
	bool normalizeEdgeIds(int& stadium_id_1, int& stadium_id_2)
	{
		if (stadium_id_1 <= 0 || stadium_id_2 <= 0 || stadium_id_1 == stadium_id_2)
			return false;

		if (stadium_id_1 > stadium_id_2)
			std::swap(stadium_id_1, stadium_id_2);

		return true;
	}
}

DistanceRepository::DistanceRepository(DatabaseManager& db_manager)
	: _db_manager(db_manager) {}

std::optional<double> DistanceRepository::getDistanceBetweenStadium(int stadium_id_1, int stadium_id_2)
{
	if (stadium_id_1 <= 0 || stadium_id_2 <= 0)
		return std::nullopt;

	if (stadium_id_1 == stadium_id_2)
		return 0.0;

	if (!normalizeEdgeIds(stadium_id_1, stadium_id_2))
		return std::nullopt;

	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return std::nullopt;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
		SELECT distance_miles
		FROM stadium_distances
		WHERE stadium_a_id = ? AND stadium_b_id = ?
		LIMIT 1;
	)SQL"))
		return std::nullopt;

	q.addBindValue(stadium_id_1);
	q.addBindValue(stadium_id_2);

	if (!q.exec() || !q.next())
		return std::nullopt;

	return q.value(0).toDouble();
}

std::optional <DistanceNode> DistanceRepository::getDistanceNodeOfStadium(int stadium_id)
{
	if (stadium_id <= 0)
		return std::nullopt;

	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return std::nullopt;

	DistanceNode node;
	node.stadium_id = stadium_id;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
		SELECT stadium_a_id, stadium_b_id, distance_miles
		FROM stadium_distances
		WHERE stadium_a_id = ? OR stadium_b_id = ?;
	)SQL"))
		return std::nullopt;

	q.addBindValue(stadium_id);
	q.addBindValue(stadium_id);

	if (!q.exec())
		return std::nullopt;

	while (q.next())
	{
		DistanceEdge edge;
		const int stadium_a_id = q.value(0).toInt();
		const int stadium_b_id = q.value(1).toInt();

		edge.from_stadium_id = stadium_id;
		edge.to_stadium_id = stadium_a_id == stadium_id ? stadium_b_id : stadium_a_id;
		edge.distance = q.value(2).toDouble();

		node.edges.push_back(edge);
	}

	if (!node.edges.empty())
		return node;

	QSqlQuery exists_q(db);
	if (!exists_q.prepare(R"SQL(
		SELECT 1
		FROM stadiums
		WHERE stadium_id = ?
		LIMIT 1;
	)SQL"))
		return std::nullopt;

	exists_q.addBindValue(stadium_id);

	if (!exists_q.exec() || !exists_q.next())
		return std::nullopt;

	return node;
}

bool DistanceRepository::updateDistanceBetweenStadiums(int stadium_id_1, int stadium_id_2, double distance)
{
	if (distance < 0.0 || !normalizeEdgeIds(stadium_id_1, stadium_id_2))
		return false;

	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return false;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
		UPDATE stadium_distances
		SET distance_miles = ?
		WHERE stadium_a_id = ? AND stadium_b_id = ?;
	)SQL"))
		return false;

	q.addBindValue(distance);
	q.addBindValue(stadium_id_1);
	q.addBindValue(stadium_id_2);

	if (!q.exec())
		return false;

	return q.numRowsAffected() > 0 || getDistanceBetweenStadium(stadium_id_1, stadium_id_2).has_value();
}

bool DistanceRepository::addDistanceBetweenStadiums(int stadium_id_1, int stadium_id_2, double distance)
{
	if (distance < 0.0 || !normalizeEdgeIds(stadium_id_1, stadium_id_2))
		return false;

	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return false;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
		INSERT INTO stadium_distances(stadium_a_id, stadium_b_id, distance_miles)
		VALUES (?, ?, ?);
	)SQL"))
		return false;

	q.addBindValue(stadium_id_1);
	q.addBindValue(stadium_id_2);
	q.addBindValue(distance);

	return q.exec();
}

bool DistanceRepository::removeDistanceBetweenStadiums(int stadium_id_1, int stadium_id_2)
{
	if (!normalizeEdgeIds(stadium_id_1, stadium_id_2))
		return false;

	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return false;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
		DELETE FROM stadium_distances
		WHERE stadium_a_id = ? AND stadium_b_id = ?;
	)SQL"))
		return false;

	q.addBindValue(stadium_id_1);
	q.addBindValue(stadium_id_2);

	if (!q.exec())
		return false;

	return q.numRowsAffected() > 0;
}
