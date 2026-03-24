#ifndef APPLICATION_H
#define APPLICATION_H

#define APP Application::instance()

class Application
{
public:
    static Application* instance();
    bool init();

private:

    Application();
    Application(Application& copy) = delete;
    ~Application();

private:
    static Application* _instance;
};

#endif // APPLICATION_H
