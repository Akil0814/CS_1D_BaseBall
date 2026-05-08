#include "main_window.h"
#include "App/application.h"
#include "ui_main_window.h"
#include "browse_window.h"
#include "main_page.h"
#include "trip_planning/trip_detail_page/trip_detail_page.h"
#include "tripplanningpage.h"

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
    QPushButton *btnTripPlanning = ui->widget->findChild<QPushButton*>("btnTripPlanning");

    if (btnTripPlanning)
    {
        connect(btnTripPlanning, &QPushButton::clicked, this, [this]() {
            TripPlanningPage *tripPlanningPage = new TripPlanningPage(this);
            tripPlanningPage->setAttribute(Qt::WA_DeleteOnClose);
            tripPlanningPage->show();
        });
    }

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

    ui->tabWidget->setStyleSheet(
    "QWidget#MLB {"
        "background-color: #0A2A66;"
        "}"
    );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionreset_triggered()
{
  bool success = APP->databaseManager()->resetDatabase();

  if(success) {
    qDebug() << "Database reset";
  }
  else
  {
    qDebug() << "Database failed to reset";
  }
}
