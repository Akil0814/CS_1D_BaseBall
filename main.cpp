#include <QApplication>
#include <iostream>

#include "main_window.h"
#include "AdminPage/AdminPage.h"
#include "AdminPage/DashboardPage.h"
#include "AdminPage/LoginPage.h"
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
         
    MainWindow w;
    test();
    w.show();
    return a.exec();
}

void test(){

    // Quick auth check for debug/demo account.
    if (APP->isAuthAvailable()){

    if (APP->isAuthAvailable()) {
        APP->authService()->idVerify("cs1d", "abc");
    }

    MainWindow w;
    w.show();

    return a.exec();
}
