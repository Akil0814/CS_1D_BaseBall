#include "main_window.h"
#include "ui_main_window.h"
#include "browse_window.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnBrowse, &QPushButton::clicked, this, [this]() {
        BrowseWindow *browseWindow = new BrowseWindow(this);
        browseWindow->show();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
