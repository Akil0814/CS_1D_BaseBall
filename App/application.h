#ifndef APPLICATION_H
#define APPLICATION_H

#define APP Application::instance()

#include <memory>
#include <QString>

#include "data_access/database_manager.h"
#include "data_access/distancere_pository.h"
#include "data_access/souvenir_repository.h"
#include "data_access/stadium_repository.h"

#include "services/auth_service.h"
#include "services/trip_planner.h"

class Application
{
public:
    // Return the global application instance.
    static Application* instance();
    // Initialize application paths, services, and repositories.
    bool init();

    // Return the stadium repository.
    StadiumRepository* stadiumRepository();
    // Return the souvenir repository if available.
    SouvenirRepository* souvenirRepository();
    // Return the distance repository if available.
    DistanceRepository* distanceRepository();

    // Return the auth service if available.
    AuthService* authService();
    // Return the trip planner if available.
    TripPlanner* tripPlanner();

    // Return whether the auth service is available.
    bool isAuthAvailable() const;
    // Return whether the souvenir repository is available.
    bool isSouvenirAvailable() const;
    // Return whether the distance repository is available.
    bool isDistanceAvailable() const;
    // Return whether the trip planner is available.
    bool isTripPlannerAvailable() const;
    // Return whether an error message is currently stored.
    bool hasError() const;
    // Return whether a warning message is currently stored.
    bool hasWarning() const;

    // Return the last recorded error message.
    const QString& lastError() const;
    // Return the last recorded warning message.
    const QString& lastWarning() const;

    // Erfan Testing
    DatabaseManager* databaseManager();

private:
    bool resolvePaths();
    void setUnavailableModuleError(const QString& module_name);

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
    QString _data_dir = {};
    QString _db_path = {};
    QString _key_path = {};

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
