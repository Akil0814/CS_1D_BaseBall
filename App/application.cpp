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
    _last_error.clear();

    if(!_db_manager)
        _db_manager = std::unique_ptr<DatabaseManager>(new DatabaseManager());

    if (!databaseManager()->init())
    {
        _last_error = databaseManager()->lastError().toStdString();
        std::cerr << _last_error << std::endl;

        if(databaseManager()->isStadiumsAvailable()==false)
            return false;
    }

    souvenir_data_available = databaseManager()->isSouvenirsAvailable();
    if (!souvenir_data_available)
        _last_error += "\nSouvenirRepository not available";

    distance_data_available = databaseManager()->isDistanceAvailable();
    if (!distance_data_available)
        _last_error += "\nDistanceRepository and TripPlanner not available";
    
    if(!_stadium_repo)
        _stadium_repo = std::unique_ptr<StadiumRepository>(new StadiumRepository());

    if (souvenir_data_available)
    {
        if (!_souvenir_repo)
        _souvenir_repo = std::unique_ptr<SouvenirRepository>(new SouvenirRepository());
    }
    else
        _souvenir_repo.reset();

    if (distance_data_available)
    {
        if(!_distance_repo)
            _distance_repo = std::unique_ptr<DistanceRepository>(new DistanceRepository());

        if (!_trip_planner)
            _trip_planner = std::unique_ptr<TripPlanner>(new TripPlanner());
    }
    else
    {
        _distance_repo.reset();
        _trip_planner.reset();
    }

    if (!_auth_service)
        _auth_service = std::unique_ptr<AuthService>(new AuthService());

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

bool Application::isSouvenirAvailable() const
{
    return souvenir_data_available && _souvenir_repo != nullptr;
}

bool Application::isDistanceAvailable() const
{
    return distance_data_available && _distance_repo != nullptr;
}

bool Application::isTripPlannerAvailable() const
{
    return distance_data_available && _trip_planner != nullptr;
}
