#include "trip_detail_page.h"
#include "ui_trip_detail_page.h"
#include "../detail_window/detail_window.h"

#include <QMessageBox>

TripDetailPage::TripDetailPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TripDetailPage)
{
    ui->setupUi(this);

    connect(ui->btnMoreInfo, &QPushButton::clicked,
            this, [this]() { openMoreInfo(); });
    connect(ui->btnPreviousStop, &QPushButton::clicked,
            this, [this]() { selectPreviousStop(); });
    connect(ui->btnNextStop, &QPushButton::clicked,
            this, [this]() { selectNextStop(); });
    connect(ui->lstTripStops, &QListWidget::currentRowChanged,
            this, [this](int) { updateNavigationButtons(); });

    updateStadiumSummary();
    updateNavigationButtons();
}

TripDetailPage::~TripDetailPage()
{
    delete ui;
}

void TripDetailPage::setCurrentStadium(const Stadium& stadium)
{
    currentStadium = stadium;
    hasCurrentStadium = true;
    updateStadiumSummary();
}

void TripDetailPage::openMoreInfo()
{
    if (!hasCurrentStadium)
    {
        QMessageBox::information(this, "No Stadium Selected",
                                 "Select a stadium before opening more information.");
        return;
    }

    DetailWindow *detailWindow = new DetailWindow(currentStadium, this);
    detailWindow->setAttribute(Qt::WA_DeleteOnClose);
    detailWindow->show();
}

void TripDetailPage::selectPreviousStop()
{
    const int currentRow = ui->lstTripStops->currentRow();
    if (currentRow > 0)
        ui->lstTripStops->setCurrentRow(currentRow - 1);
}

void TripDetailPage::selectNextStop()
{
    const int currentRow = ui->lstTripStops->currentRow();
    const int lastRow = ui->lstTripStops->count() - 1;

    if (currentRow >= 0 && currentRow < lastRow)
        ui->lstTripStops->setCurrentRow(currentRow + 1);
}

void TripDetailPage::updateNavigationButtons()
{
    const int count = ui->lstTripStops->count();
    const int currentRow = ui->lstTripStops->currentRow();
    const bool hasSelection = currentRow >= 0 && currentRow < count;

    ui->btnPreviousStop->setEnabled(hasSelection && currentRow > 0);
    ui->btnNextStop->setEnabled(hasSelection && currentRow < count - 1);
}

void TripDetailPage::updateStadiumSummary()
{
    if (!hasCurrentStadium)
    {
        ui->lblSelectedStadium->setText("No stadium selected");
        ui->lblTeamNameValue->setText("N/A");
        ui->lblLeagueValue->setText("N/A");
        ui->lblLocationValue->setText("N/A");
        ui->btnMoreInfo->setEnabled(false);
        return;
    }

    ui->lblSelectedStadium->setText(currentStadium.stadium_name);
    ui->lblTeamNameValue->setText(currentStadium.team_name);
    ui->lblLeagueValue->setText(currentStadium.league);
    ui->lblLocationValue->setText(currentStadium.location);
    ui->btnMoreInfo->setEnabled(true);
}
