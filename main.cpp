#include <QApplication>
#include <iostream>

#include "main_window.h"
#include"App/application.h"

void test();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    test();
    w.show();
    return a.exec();
}

void test()
{
    APP->init();
    APP->authService()->idVerify("cs1d", "abc");
    std::cout << "testing" << std::endl;
}
