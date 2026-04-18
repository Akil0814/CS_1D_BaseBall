#include "souvenir_repository.h"
#include <QSqlDatabase>
#include <QSqlQuery>

SouvenirRepository::SouvenirRepository(DatabaseManager& db_manager)
	: _db_manager(db_manager) {}

std::vector<Souvenir> SouvenirRepository::getSouvenirsByStadiumID(int stadium_id)
{
	std::vector<Souvenir> tmp;

	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return tmp;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
        SELECT
            souvenir_id,
            stadium_id,
            name,
            price
        FROM souvenirs
        WHERE stadium_id = ?
        ORDER BY name ASC, souvenir_id ASC;
    )SQL"))
		return tmp;

	q.addBindValue(stadium_id);

	if (!q.exec())
		return tmp;

	while (q.next())
	{
		Souvenir souvenir;
		souvenir.souvenir_id = q.value("souvenir_id").toInt();
		souvenir.owner_stadium_id = q.value("stadium_id").toInt();
		souvenir.name = q.value("name").toString();
		souvenir.price = q.value("price").toDouble();
		tmp.push_back(souvenir);
	}

	return tmp;
}
std::optional<Souvenir> SouvenirRepository::getSouvenirByID(int souvenir_id)
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return std::nullopt;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
        SELECT
            souvenir_id,
            stadium_id,
            name,
            price
        FROM souvenirs
        WHERE souvenir_id = ?
        LIMIT 1;
    )SQL"))
		return std::nullopt;

	q.addBindValue(souvenir_id);

	if (!q.exec())
		return std::nullopt;

	if (!q.next())
		return std::nullopt;

	Souvenir souvenir;
	souvenir.souvenir_id = q.value("souvenir_id").toInt();
	souvenir.owner_stadium_id = q.value("stadium_id").toInt();
	souvenir.name = q.value("name").toString();
	souvenir.price = q.value("price").toDouble();

	return souvenir;
}

std::optional<int> SouvenirRepository::getSouvenirID(int stadium_id, const QString& souvenir_name)
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return std::nullopt;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
        SELECT souvenir_id
        FROM souvenirs
        WHERE stadium_id = ? AND name = ?
        LIMIT 1;
    )SQL"))
		return std::nullopt;

	q.addBindValue(stadium_id);
	q.addBindValue(souvenir_name);

	if (!q.exec())
		return std::nullopt;

	if (!q.next())
		return std::nullopt;

	return q.value("souvenir_id").toInt();
}

bool SouvenirRepository::addSouvenir(int stadium_id, const Souvenir& souvenir)
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return false;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
        INSERT INTO souvenirs
        (
            stadium_id,
            name,
            price
        )
        VALUES (?, ?, ?);
    )SQL"))
		return false;

	q.addBindValue(stadium_id);
	q.addBindValue(souvenir.name);
	q.addBindValue(souvenir.price);

	return q.exec();

}

bool SouvenirRepository::updateSouvenir(const Souvenir& souvenir)
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return false;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
        UPDATE souvenirs
        SET
            stadium_id = ?,
            name = ?,
            price = ?
        WHERE souvenir_id = ?;
    )SQL"))
		return false;

	q.addBindValue(souvenir.owner_stadium_id);
	q.addBindValue(souvenir.name);
	q.addBindValue(souvenir.price);
	q.addBindValue(souvenir.souvenir_id);

	if (!q.exec())
		return false;

	return q.numRowsAffected() > 0 || getSouvenirByID(souvenir.souvenir_id).has_value();

}

bool SouvenirRepository::updateSouvenirPrice(int souvenir_id, double new_price)
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return false;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
        UPDATE souvenirs
        SET
            price = ?
        WHERE souvenir_id = ?;
    )SQL"))
		return false;

	q.addBindValue(new_price);
	q.addBindValue(souvenir_id);

	if (!q.exec())
		return false;

	return q.numRowsAffected() > 0 || getSouvenirByID(souvenir_id).has_value();
}

bool SouvenirRepository::deleteSouvenir(int souvenir_id)
{
	QSqlDatabase db = _db_manager.getDatabaseObj();
	if (!db.isValid() || !db.isOpen())
		return false;

	QSqlQuery q(db);
	if (!q.prepare(R"SQL(
        DELETE FROM souvenirs
        WHERE souvenir_id = ?;
    )SQL"))
		return false;

	q.addBindValue(souvenir_id);

	if (!q.exec())
		return false;

	return q.numRowsAffected() > 0;
}
