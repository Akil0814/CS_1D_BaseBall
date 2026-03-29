#include <QApplication>
#include <iostream>

#include "main_window.h"
#include"App/application.h"

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

    if (APP->isAuthAvailable()){

        APP->authService()->idVerify("cs1d", "abc");
        //or
        //auto* auth = APP->authService();
        //auth->idVerify("cs1d", "abc");
    }

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
}
