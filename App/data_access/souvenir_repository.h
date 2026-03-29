#ifndef SOUVENIR_REPOSITORY_H
#define SOUVENIR_REPOSITORY_H

#include <optional>
#include "../../data_types.h"
#include "database_manager.h"

class Application;

class SouvenirRepository
{
    friend class Application;

public:


private:
    SouvenirRepository(DatabaseManager& db_manager);
    SouvenirRepository(const SouvenirRepository& copy) = delete;
    SouvenirRepository& operator=(const SouvenirRepository& copy) = delete;
    SouvenirRepository(SouvenirRepository&& move) = delete;
    SouvenirRepository& operator=(SouvenirRepository&& move) = delete;

private:
    DatabaseManager& _db_manager;
};

#endif // SOUVENIR_REPOSITORY_H
