#include <QApplication>
#include <iostream>

#include "main_window.h"
#include "AdminPage/AdminPage.h"
#include "AdminPage/DashboardPage.h"
#include "AdminPage/LoginPage.h"
#include "App/application.h"
#include "main_page/main_page.h"

void test();

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

 void test()
{
     QString path1 = "";
     QString path2 = "";
     if (!APP->databaseManager()->importStadiumsFromFile(path1))
         std::cerr << "false 1" << std::endl;
     if(!APP->databaseManager()->importDistancesFromFile(path2))
        std::cerr << "false 2" << std::endl;

     if (APP->hasWarning())
         std::cout << APP->lastWarning().toStdString() << std::endl;
}
