#include "browse_window.h"
#include "detail_window.h"
#include "ui_browse_window.h"

BrowseWindow::BrowseWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::browse_window)
{
    ui->setupUi(this);

    connect(ui->btnMoreDetail, &QPushButton::clicked, this, [this]() {
        DetailWindow *detailWindow = new DetailWindow(this);
        detailWindow->show();
    });

    connect(ui->btnBackToMain, &QPushButton::clicked, this, [this]() {
        this->close();
    });
}

BrowseWindow::~BrowseWindow()
{
    delete ui;
}
