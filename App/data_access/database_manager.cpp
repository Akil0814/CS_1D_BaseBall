#include "database_manager.h"

DatabaseManager::DatabaseManager(){}

bool DatabaseManager::init()
{
	return true;

}

bool DatabaseManager::resetDatabase(bool remove_backup_if_success)
{
	return true;
}

bool DatabaseManager::isOpen() const
{
	return true;
}

const std::string DatabaseManager::lastError()
{
	return "error";
}


