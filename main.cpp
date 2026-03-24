#include <QApplication>
#include <iostream>

#include "main_window.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.show();
    return a.exec();
}

void test()
{
    std::cout << "testing" << std::endl;
}
