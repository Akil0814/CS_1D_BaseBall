#include <QApplication>
#include <iostream>

#include "main_window.h"
#include"App/application.h"

void test();

int main(int argc, char *argv[]){

    QApplication a(argc, argv);
    MainWindow w;
    if (!APP->init()){
        std::cerr<< "init failed:" << APP->lastError() << std::endl;
        throw std::runtime_error("init failed:");
    }

    test();
    w.show();
    return a.exec();
}

void test(){

    std::cout << "testing" << std::endl;
}
