#include "application.h"

#include <iostream>

Application* Application::_instance = nullptr;

Application* Application::instance()
{
    if (_instance == nullptr)
    {
        _instance = new Application();
    }

    std::cout << "test" << std::endl;

    return _instance;
}

Application::Application() = default;

Application::~Application() = default;

bool Application::init()
{
    _db_manager = std::make_unique<DatabaseManager>();

    _stadium_repo = std::make_unique<StadiumRepository>();
    _souvenir_repo = std::make_unique<SouvenirRepository>();
    _distance_repo = std::make_unique<DistanceRepository>();

    _auth_service = std::make_unique<AuthService>();
    _trip_planner = std::make_unique<TripPlanner>();

    return true;
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