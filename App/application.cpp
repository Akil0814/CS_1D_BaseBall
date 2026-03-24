#include "application.h"

#include <iostream>

Application* Application::_instance = nullptr;

Application* Application::instance()
{
    if (_instance == nullptr)
    {
        _instance = new Application();
    }

    std::cout << "working" << std::endl;

    return _instance;
}

bool Application::init()
{
    return true;
}

Application::~Application()
{

}

Application::Application()
{

}
