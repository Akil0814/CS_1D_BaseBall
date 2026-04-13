#include <QApplication>
#include <iostream>

#include "main_window.h"
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
    main_page mainPageVar;
    mainPageVar.show();
    test();
    // w.show();
    return a.exec();
}

void test(){

    if (APP->isAuthAvailable()){

        APP->authService()->idVerify("cs1d", "abc");
        //or
        //auto* auth = APP->authService();
        //auth->idVerify("cs1d", "abc");
    }
}
