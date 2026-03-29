#include "distancere_pository.h"
#include <QSqlDatabase>
#include <QSqlQuery>

DistanceRepository::DistanceRepository(DatabaseManager& db_manager)
	: _db_manager(db_manager) {}
