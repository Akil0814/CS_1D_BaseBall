#include <QApplication>
#include <iostream>

#include "main_window.h"
#include "AdminPage/adminpage.h"
#include "AdminPage/dashboardpage.h"
#include "AdminPage/loginpage.h"
#include "App/application.h"
#include "main_page/main_page.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!APP->init()) {
        std::cerr << "init failed: " << APP->lastError().toStdString() << std::endl;
        throw std::runtime_error("init failed");
    }

    if (APP->hasWarning())
        std::cout << APP->lastWarning().toStdString() << std::endl;

    if (APP->isAuthAvailable()) {
        APP->authService()->idVerify("cs1d", "abc");
    }

    MainWindow w;
    w.show();

    return a.exec();
}
