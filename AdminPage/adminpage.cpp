//
// Created by Erfan Tavassoli on 4/6/26.
//

// You may need to build the project (run Qt uic code generator) to get "ui_AdminPage.h" resolved

#include "adminpage.h"
#include "ui_AdminPage.h"


AdminPage::AdminPage(QWidget *parent) :
    QWidget(parent), ui(new Ui::AdminPage) {
    ui->setupUi(this);

    // set stack widget to login page
    ui->AdminPageStack->setCurrentIndex(0);

    connect(ui->LoginPageWidget, &LoginPage::loginAccepted, this, [this]() {
        // Change the StackedWidget to the Admin page (usually index 1)
        ui->AdminPageStack->setCurrentIndex(1);
        qDebug() << "Login successful! Switching to Admin Page.";
    });
}

AdminPage::~AdminPage() {
    delete ui;
}
