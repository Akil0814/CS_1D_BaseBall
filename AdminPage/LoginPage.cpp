//
// Created by Erfan Tavassoli on 4/6/26.
//

// You may need to build the project (run Qt uic code generator) to get "ui_LoginPage.h" resolved

#include "LoginPage.h"
#include "ui_LoginPage.h"
#include "App/services/auth_service.h"
#include <QMessageBox>

#include "App/application.h"


LoginPage::LoginPage(QWidget *parent) :
    QWidget(parent), ui(new Ui::LoginPage) {
    ui->setupUi(this);

    // ability to press enter/return to input the login request
    connect(ui->passwordField, &QLineEdit::returnPressed, this, &LoginPage::on_buttonBox_accepted);
    connect(ui->usernameField, &QLineEdit::returnPressed, this, &LoginPage::on_buttonBox_accepted);
}

LoginPage::~LoginPage() {
    delete ui;
}

// TODO: implement proper admin password checking
void LoginPage::on_buttonBox_accepted() {
    QString username = ui->usernameField->text();
    QString password = ui->passwordField->text();

    // TODO: remove debug login
    if (APP->isAuthAvailable()) {
        if (username == "debug" && password == "debug") {
            emit loginAccepted();
        } else if (APP->authService()->idVerify(username.toStdString(), password.toStdString())) {
            emit loginAccepted();
        } else {
            QMessageBox::warning(this, "Login Failed", "Incorrect username or password.");
        }
    } else {
        qDebug() << "auth is not available";
    }
}

void LoginPage::on_buttonBox_rejected() const {
    ui->passwordField->clear();
    ui->usernameField->clear();
}
