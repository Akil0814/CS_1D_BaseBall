#include "souvenir_repository.h"
#include <QSqlDatabase>
#include <QSqlQuery>

SouvenirRepository::SouvenirRepository(DatabaseManager& db_manager)
	: _db_manager(db_manager) {}

std::vector<Souvenir> SouvenirRepository::getSouvenirsByStadiumID(int stadium_id)
{
	std::vector<Souvenir> tmp;
	return tmp;
}
std::optional<Souvenir> SouvenirRepository::getSouvenirByID(int souvenir_id)
{
	return std::nullopt;
}

std::optional<int> SouvenirRepository::getSouvenirID(int stadium_id, const QString& souvenir_name)
{
	return std::nullopt;
}

bool SouvenirRepository::addSouvenir(int stadium_id, const Souvenir& souvenir)
{
	return false;

}

bool SouvenirRepository::updateSouvenir(const Souvenir& souvenir)
{
	return false;

}

bool SouvenirRepository::updateSouvenirPrice(int souvenir_id, double new_price)
{
	return false;
}

bool SouvenirRepository::deleteSouvenir(int souvenir_id)
{
	return false;
}