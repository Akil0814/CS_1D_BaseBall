#include "main_window.h"
#include "ui_main_window.h"
#include "browse_window.h"
#include "adminpage.h"
#include "main_page.h"

#include <QPushButton>
#include <QTabWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Buttons live inside the embedded main_page widget
    QPushButton *btnBrowse = ui->widget->findChild<QPushButton*>("btnBrowse");
    QPushButton *btnAdmin = ui->widget->findChild<QPushButton*>("btnAdmin");

    if (btnBrowse)
    {
        connect(btnBrowse, &QPushButton::clicked, this, [this]() {
            BrowseWindow *browseWindow = new BrowseWindow(this);
            browseWindow->show();
        });
    }

    if (btnAdmin)
    {
        connect(btnAdmin, &QPushButton::clicked, this, [this]() {
            ui->tabWidget->setCurrentWidget(ui->AdminTab);
        });
    }
    // connect(ui->btnBrowse, &QPushButton::clicked, this, [this]() {
    //     BrowseWindow *browseWindow = new BrowseWindow(this);
    //     browseWindow->show();
    // });
}

MainWindow::~MainWindow()
{
    delete ui;
}
