#ifndef DISTANCERE_POSITORY_H
#define DISTANCERE_POSITORY_H

#include <optional>
#include "../../data_types.h"
#include "database_manager.h"

class Application;

class DistanceRepository
{
    friend class Application;

public:


private:
    DistanceRepository(DatabaseManager& db_manager);
    DistanceRepository(const DistanceRepository& copy) = delete;
    DistanceRepository& operator=(const DistanceRepository& copy) = delete;
    DistanceRepository(DistanceRepository&& move) = delete;
    DistanceRepository& operator=(DistanceRepository&& move) = delete;

private:
    DatabaseManager& _db_manager;
};

#endif // DISTANCERE_POSITORY_H
