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
    std::vector<Souvenir> getSouvenirsByStadiumID(int stadium_id);
    std::optional<Souvenir> getSouvenirByID(int souvenir_id);
    std::optional<int> getSouvenirID(int stadium_id, const QString& souvenir_name);

    bool addSouvenir(int stadium_id, const Souvenir& souvenir);

    bool updateSouvenir(const Souvenir& souvenir);
    bool updateSouvenirPrice(int souvenir_id, double new_price);

    bool deleteSouvenir(int souvenir_id);


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
