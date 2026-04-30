#include <QApplication>
#include <iostream>

#include "main_window.h"
#include "AdminPage/AdminPage.h"
#include "AdminPage/DashboardPage.h"
#include "AdminPage/LoginPage.h"
#include "App/application.h"
#include "main_page/main_page.h"

//void test();

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
    //test();

    // Uncomment to rebuild the database.
    // The existing database will be automatically backed up as a .bak file before reset.
    //APP->databaseManager()->resetDatabase();

    w.show();
    return a.exec();
}

 void test()
{
     // Use a full local path.
     QString path1 = "W:/Coding/vCS_1D_BaseBall/assets/MLB_Information_Expansion.csv";
     QString path2 = "W:/Coding/vCS_1D_BaseBall/assets/Distance_between_new_team.csv";

     // Expect a full local file path here.
     if (!APP->databaseManager()->importStadiumsFromFile(path1))
     {
         std::cerr << APP->databaseManager()->lastError().toStdString() << std::endl;
         std::cerr << APP->databaseManager()->lastWarning().toStdString() << std::endl;
     }

     if (!APP->databaseManager()->importDistancesFromFile(path2))
     {
         std::cerr << APP->databaseManager()->lastError().toStdString() << std::endl;
         std::cerr << APP->databaseManager()->lastWarning().toStdString() << std::endl;
     }
}
