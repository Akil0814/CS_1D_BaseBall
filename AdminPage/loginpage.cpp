//
// Created by Erfan Tavassoli on 4/6/26.
//

// You may need to build the project (run Qt uic code generator) to get "ui_LoginPage.h" resolved

#include "loginpage.h"
#include "ui_LoginPage.h"


LoginPage::LoginPage(QWidget *parent) :
    QWidget(parent), ui(new Ui::LoginPage) {
    ui->setupUi(this);
}

LoginPage::~LoginPage() {
    delete ui;
}

// TODO: implement proper admin password checking
void LoginPage::on_buttonBox_accepted() {
    bool isAuthenticated = true;

    if (isAuthenticated) {
        emit loginAccepted();
    }
}

void LoginPage::on_buttonBox_rejected() const {
    ui->passwordField->clear();
    ui->usernameField->clear();
}
