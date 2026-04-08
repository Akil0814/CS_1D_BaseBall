#include "distancere_pository.h"
#include <QSqlDatabase>
#include <QSqlQuery>

DistanceRepository::DistanceRepository(DatabaseManager& db_manager)
	: _db_manager(db_manager) {}

std::optional<double> DistanceRepository::getDistanceBetweenStadium(int stadium_id_1, int stadium_id_2)
{
	return std::nullopt;
}
std::optional <DistanceNode> DistanceRepository::getDistanceNodeOfStadium(int stadium_id)
{
	return std::nullopt;
}

bool DistanceRepository::updateDistanceBetweenStadiums(int stadium_id_1, int stadium_id_2, double distance)
{
	return false;
}

bool DistanceRepository::addDistanceBetweenStadiums(int stadium_id_1, int stadium_id_2, double distance)
{
	return false;
}

bool DistanceRepository::removeDistanceBetweenStadiums(int stadium_id_1, int stadium_id_2)
{
	return false;
}
