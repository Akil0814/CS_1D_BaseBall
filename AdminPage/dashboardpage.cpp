//
// Created by Erfan Tavassoli on 4/6/26.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DashboardPage.h" resolved

#include "dashboardpage.h"
#include "ui_DashboardPage.h"


DashboardPage::DashboardPage(QWidget *parent) :
    QWidget(parent), ui(new Ui::DashboardPage) {
    ui->setupUi(this);
}

DashboardPage::~DashboardPage() {
    delete ui;
}
