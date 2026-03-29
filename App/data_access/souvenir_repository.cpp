#include "souvenir_repository.h"
#include <QSqlDatabase>
#include <QSqlQuery>

SouvenirRepository::SouvenirRepository(DatabaseManager& db_manager)
	: _db_manager(db_manager) {}