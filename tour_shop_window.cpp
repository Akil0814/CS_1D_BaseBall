#include "tour_shop_window.h"
#include "ui_tour_shop_window.h"
#include "mapwindow.h"
#include <QPushButton>

TourShopWindow::TourShopWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::tour_shop_window)
{
    ui->setupUi(this);

    setWindowTitle("Tour and Shop");

    this->setStyleSheet(
        "QMainWindow {"
        "background-color: #f4f6f8;"
        "}"
        "QLabel {"
        "color: #1f2937;"
        "font-weight: bold;"
        "}"
        "QPushButton {"
        "background-color: white;"
        "border: 2px solid #cfd8e3;"
        "border-radius: 12px;"
        "padding: 10px;"
        "font-size: 12pt;"
        "font-weight: bold;"
        "color: #1f2937;"
        "}"
        "QPushButton:hover {"
        "background-color: #eef4ff;"
        "border: 2px solid #7aa7ff;"
        "}"
        "QPushButton:pressed {"
        "background-color: #dbeafe;"
        "}"
        );

    connect(ui->btnBackToMain, &QPushButton::clicked, this, &TourShopWindow::close);
    connect(ui->btnCustomVisitCustomStart, &QPushButton::clicked, this, [this]() {
        MapWindow *mapWindow = new MapWindow(this);
        mapWindow->show();
    });

    connect(ui->btnCustomVisitDodger, &QPushButton::clicked, this, [this]() {
        MapWindow *mapWindow = new MapWindow(this);
        mapWindow->show();
    });
}

TourShopWindow::~TourShopWindow()
{
    delete ui;
}
