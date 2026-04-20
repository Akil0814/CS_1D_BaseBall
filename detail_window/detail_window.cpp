#include "detail_window.h"
#include "ui_detail_window.h"
#include <QPushButton>

DetailWindow::DetailWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::detail_window)
{
    ui->setupUi(this);

    connect(ui->btnClose, &QPushButton::clicked, this, &DetailWindow::close);
}

DetailWindow::~DetailWindow()
{
    delete ui;
}
