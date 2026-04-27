#include <QApplication>
#include <iostream>

#include "main_window.h"
#include "AdminPage/AdminPage.h"
#include "AdminPage/DashboardPage.h"
#include "AdminPage/LoginPage.h"
#include"App/application.h"
#include "main_page/main_page.h"

void test();

int main(int argc, char *argv[]){

    QApplication a(argc, argv);

    if (!APP->init()){
        std::cerr<< "init failed:" << APP->lastError().toStdString() << std::endl;
        throw std::runtime_error("init failed:");
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

        APP->authService()->idVerify("cs1d", "abc");
        //or
        //auto* auth = APP->authService();
        //auth->idVerify("cs1d", "abc");
    }

    // Fetch one stadium by id and print a few key fields
    auto tmp=APP->stadiumRepository()->getStadiumByID(3);
    //auto tmp = APP->stadiumRepository()->getStadiumByID(999);
    if (tmp.has_value())
    {
        const auto& stadium_t = tmp.value();

        std::cout << "stadium_id: " << stadium_t.stadium_id << '\n';
        std::cout << "team_name: " << stadium_t.team_name.toStdString() << '\n';
        std::cout << "stadium_name: " << stadium_t.stadium_name.toStdString() << '\n';
        std::cout << "location: " << stadium_t.location.toStdString() << '\n';
    }
    else
        std::cout << "No stadium found\n";

    // Fetch all stadiums with sorting/filter and print a simple list
    auto stadiums = APP->stadiumRepository()->getAllStadiums(
        StadiumRepository::StadiumSortBy::DateOpened,
        StadiumRepository::LeagueFilter::All
    );

    std::cout << "\ngetAllStadiums result count: " << stadiums.size() << '\n';

    for (const auto& stadium : stadiums)
    {
        std::cout << "--------------------\n";
        std::cout << "stadium_id: " << stadium.stadium_id << '\n';
        std::cout << "team_name: " << stadium.team_name.toStdString() << '\n';
        std::cout << "stadium_name: " << stadium.stadium_name.toStdString() << '\n';
        std::cout << "location: " << stadium.location.toStdString() << '\n';
    }
}
