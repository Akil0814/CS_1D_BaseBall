#ifndef STADIUM_REPOSITORY_H
#define STADIUM_REPOSITORY_H

#include <optional>
#include "../../data_types.h"
#include "database_manager.h"

class Application;

class StadiumRepository
{
    friend class Application;
public:
    enum class StadiumSortBy
    {
        TeamName,
        StadiumName,
        DateOpened,
        SeatingCapacity,
        Typology
    };

    enum class LeagueFilter
    {
        All,
        American,
        National
    };

public:

    std::vector<Stadium> getAllStadiums(StadiumSortBy sort_by, LeagueFilter league = LeagueFilter::All);

    std::optional<Stadium> getStadiumByID(int stadium_id);
    std::optional<Stadium> getStadiumByStadiumName(const QString& stadium_name);
    std::optional<Stadium> getStadiumByTeamName(const QString& team_name);

    std::optional<int> getStadiumID(const QString& stadium_name);
    std::optional<QString> getStadiumName(int stadium_id);


private:
    Stadium buildStadiumFromQuery(const QSqlQuery& q) const;

    StadiumRepository(DatabaseManager& db_manager);
    StadiumRepository(const StadiumRepository& copy) = delete;
    StadiumRepository& operator=(const StadiumRepository& copy) = delete;
    StadiumRepository(StadiumRepository&& move) = delete;
    StadiumRepository& operator=(StadiumRepository&& move) = delete;

private:
    DatabaseManager& _db_manager;
};

#endif // STADIUM_REPOSITORY_H
