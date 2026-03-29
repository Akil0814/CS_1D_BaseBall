#include "application.h"
#include "utils/message_utils.h"

#include <QDir>
#include <QFileInfo>

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
    _last_warning.clear();

    if (!resolvePaths())
        return false;

    const QString path_warning = _last_warning.trimmed();
    QStringList warnings;

    if(!_db_manager)
        _db_manager = std::unique_ptr<DatabaseManager>(new DatabaseManager());
    _db_manager->setDatabasePath(_db_path);

    const bool database_init_ok = databaseManager()->init();
    const QString database_error = databaseManager()->lastError().trimmed();

    if (!database_init_ok)
    {
        if(databaseManager()->isStadiumModuleAvailable()==false)
        {
            _last_error = database_error.isEmpty()
                ? "database initialization failed"
                : database_error;
            return false;
        }

        MessageUtils::appendUniqueMessage(warnings, database_error);
    }

    MessageUtils::appendUniqueMessage(warnings, path_warning);

    if (databaseManager()->hasWarning())
        MessageUtils::appendUniqueMessage(warnings, databaseManager()->lastWarning());

    souvenir_data_available = databaseManager()->isSouvenirModuleAvailable();
    if (!souvenir_data_available)
        MessageUtils::appendUniqueMessage(warnings, "SouvenirRepository not available");

    distance_data_available = databaseManager()->isDistanceModuleAvailable();
    if (!distance_data_available)
        MessageUtils::appendUniqueMessage(warnings, "DistanceRepository and TripPlanner not available");
    
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

    if (_key_path.trimmed().isEmpty())
    {
        _auth_service.reset();
        MessageUtils::appendUniqueMessage(warnings, "AuthService not available");
    }
    else
    {
        if (!_auth_service)
            _auth_service = std::unique_ptr<AuthService>(new AuthService());
        _auth_service->setKeyPath(_key_path);
    }

    _last_error.clear();
    _last_warning = MessageUtils::joinMessages(warnings);

    return true;
}

bool Application::resolvePaths()
{
    _data_dir.clear();
    _db_path.clear();
    _key_path.clear();

    QDir dir(QDir::current());

    for (int i = 0; i < 8; ++i)
    {
        const QString app_data_candidate = dir.filePath("App/data");
        if (QDir(app_data_candidate).exists())
        {
            _data_dir = QDir(app_data_candidate).absolutePath();
            break;
        }

        const QString data_candidate = dir.filePath("data");
        if (QDir(data_candidate).exists())
        {
            _data_dir = QDir(data_candidate).absolutePath();
            break;
        }

        if (!dir.cdUp())
            break;
    }

    if (_data_dir.isEmpty())
    {
        _last_error = "failed to locate data directory";
        return false;
    }

    _db_path = QDir(_data_dir).filePath("BaseBall_data.db");

    const QString candidate_key_path = QDir(_data_dir).filePath("key.dat");
    if (QFileInfo::exists(candidate_key_path))
        _key_path = candidate_key_path;
    else
        _last_warning = "AuthService key file not found: " + candidate_key_path;

    return true;
}

const QString& Application::lastError() const
{
    return _last_error;
}

const QString& Application::lastWarning() const
{
    return _last_warning;
}

bool Application::hasError() const
{
    return !_last_error.trimmed().isEmpty();
}

bool Application::hasWarning() const
{
    return !_last_warning.trimmed().isEmpty();
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

bool Application::isAuthAvailable() const
{
    return _auth_service != nullptr;
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
