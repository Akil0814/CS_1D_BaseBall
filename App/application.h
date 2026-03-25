#ifndef APPLICATION_H
#define APPLICATION_H

#define APP Application::instance()

#include <memory>

#include "data_access/database_manager.h"
#include "data_access/distancere_pository.h"
#include "data_access/souvenir_repository.h"
#include "data_access/stadium_repository.h"

#include "services/auth_service.h"
#include "services/trip_planner.h"

class Application
{
public:
    static Application* instance();
    bool init();

    DatabaseManager* databaseManager() const;
    StadiumRepository* stadiumRepository() const;
    SouvenirRepository* souvenirRepository() const;
    DistanceRepository* distanceRepository() const;

    AuthService* authService() const;
    TripPlanner* tripPlanner() const;

    const std::string& lastError() const;

private:
    Application();
    Application(const Application& copy) = delete;
    Application& operator=(const Application& copy) = delete;
    Application(Application&& move) = delete;
    Application& operator=(Application&& move) = delete;
    ~Application();

private:
    std::string _last_error = {};


private:
    std::unique_ptr<DatabaseManager> _db_manager;

    std::unique_ptr<StadiumRepository> _stadium_repo;
    std::unique_ptr<SouvenirRepository> _souvenir_repo;
    std::unique_ptr<DistanceRepository> _distance_repo;

    std::unique_ptr<AuthService> _auth_service;
    std::unique_ptr<TripPlanner> _trip_planner;
};

#endif // APPLICATION_H
