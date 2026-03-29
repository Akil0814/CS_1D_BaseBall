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

    std::cout << "testing" << std::endl;
}
