#include "mapwindow.h"
#include "ui_mapwindow.h"
#include "App/application.h"
#include <QResizeEvent>
#include <QPushButton>
#include <QPixmap>

MapWindow::MapWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MapWindow)
{
    ui->setupUi(this);

    setWindowTitle("Map");

    QString path = APP->assetsDir();
    QPixmap mapPixmap(path + "/images/map.png");

    ui->lblMap->setPixmap(
        mapPixmap.scaled(ui->lblMap->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)
        );
    ui->lblMap->setAlignment(Qt::AlignCenter);

    connect(ui->btnBackToMain, &QPushButton::clicked, this, &MapWindow::close);
}

MapWindow::~MapWindow()
{
    delete ui;
}

void MapWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    QString path = APP->assetsDir();
    QPixmap mapPixmap(path + "/images/map.png");

    ui->lblMap->setPixmap(
        mapPixmap.scaled(ui->lblMap->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)
        );
}
