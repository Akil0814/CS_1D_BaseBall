#include "application.h"

Application* Application::instance()
{
    static Application instance;
    return &instance;
}

Application::Application() = default;

Application::~Application() = default;

bool Application::init()
{
    _db_manager = std::unique_ptr<DatabaseManager>(new DatabaseManager());

    if (databaseManager()->init())
    {
        _last_error = databaseManager()->lastError();
        std::cerr << _last_error << std::endl;
        return false;
    }

    _stadium_repo = std::unique_ptr<StadiumRepository>(new StadiumRepository());
    _souvenir_repo = std::unique_ptr<SouvenirRepository>(new SouvenirRepository());
    _distance_repo = std::unique_ptr<DistanceRepository>(new DistanceRepository());

    _auth_service = std::unique_ptr<AuthService>(new AuthService());
    _trip_planner = std::unique_ptr<TripPlanner>(new TripPlanner());

    return true;
}

const std::string& Application::lastError() const
{
    return _last_error;
}


DatabaseManager* Application::databaseManager() const
{
    return _db_manager.get();
}

StadiumRepository* Application::stadiumRepository() const
{
    return _stadium_repo.get();
}

SouvenirRepository* Application::souvenirRepository() const
{
    return _souvenir_repo.get();
}

DistanceRepository* Application::distanceRepository() const
{
    return _distance_repo.get();
}

AuthService* Application::authService() const
{
    return _auth_service.get();
}

TripPlanner* Application::tripPlanner() const
{
    return _trip_planner.get();
}
