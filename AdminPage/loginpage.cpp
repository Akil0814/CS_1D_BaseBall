//
// Created by Erfan Tavassoli on 4/6/26.
//

// You may need to build the project (run Qt uic code generator) to get "ui_LoginPage.h" resolved

#include "loginpage.h"
#include "ui_LoginPage.h"
#include "App/services/auth_service.h"
#include <QMessageBox>


LoginPage::LoginPage(QWidget *parent) :
    QWidget(parent), ui(new Ui::LoginPage) {
    ui->setupUi(this);
}

LoginPage::~LoginPage() {
    delete ui;
}

// TODO: implement proper admin password checking
void LoginPage::on_buttonBox_accepted() {
    QString username = ui->usernameField->text();
    QString password = ui->passwordField->text();

    AuthService auth;

    // TODO: remove debug login
    if (username == "debug" && password == "debug") {
        emit loginAccepted();
    } else if (auth.idVerify(username.toStdString(), password.toStdString())) {
        emit loginAccepted();
    } else {
        QMessageBox::warning(this, "Login Failed", "Incorrect username or password.");
        // QMessageBox::StandardButton reply;
        // reply = QMessageBox::warning(this, "Invalid", "Incorrect username or password",
                                        // QMessageBox::Ok);
    }
}

void LoginPage::on_buttonBox_rejected() const {
    ui->passwordField->clear();
    ui->usernameField->clear();
}
