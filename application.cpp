#include "application.h"

Application* Application::_instance = nullptr;

Application* Application::instansce()
{
    if (_instance == nullptr)
    {
        _instance = new Application();
    }

    return _instance;
}

Application::init()
{

}


Application::Application()
{

}
