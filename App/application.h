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

    bool isSouvenirAvailable() const;
    bool isDistanceAvailable() const;
    bool isTripPlannerAvailable() const;

    const QString& lastError() const;
    const QString& lastWarning() const;

private:
    Application();
    Application(const Application& copy) = delete;
    Application& operator=(const Application& copy) = delete;
    Application(Application&& move) = delete;
    Application& operator=(Application&& move) = delete;
    ~Application();

private:
    QString _last_error = {};
    QString _last_warning = {};

    bool souvenir_data_available = false;
    bool distance_data_available = false;

private:
    std::unique_ptr<DatabaseManager> _db_manager;

    std::unique_ptr<StadiumRepository> _stadium_repo;
    std::unique_ptr<SouvenirRepository> _souvenir_repo;
    std::unique_ptr<DistanceRepository> _distance_repo;

    std::unique_ptr<AuthService> _auth_service;
    std::unique_ptr<TripPlanner> _trip_planner;
};

#endif // APPLICATION_H
