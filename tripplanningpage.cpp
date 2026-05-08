#include "tripplanningpage.h"
#include "ui_tripplanningpage.h"
#include "trip_detail_page/trip_detail_page.h"

#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <vector>
#include <algorithm>

#include "App/application.h"

TripPlanningPage::TripPlanningPage(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TripPlanningPage)
{
    ui->setupUi(this);

    setWindowTitle("Trip Planning");

    ui->cmbTripType->clear();
    ui->cmbTripType->addItem("Shortest Trip from Dodger to Selected Team");
    ui->cmbTripType->addItem("Custom Ordered Trip");
    ui->cmbTripType->addItem("Visit All Teams from Marlins");
    ui->cmbTripType->addItem("Custom Efficient Trip");
    ui->cmbTripType->addItem("Minimum Spanning Tree (MST)");
    ui->cmbTripType->addItem("DFS from Oracle Park");
    ui->cmbTripType->addItem("BFS from Target Field");

    loadStadiumAndTeamData();

    ui->btnContinueToTripDetail->setEnabled(false);

    connect(ui->btnAddTeam, &QPushButton::clicked, this, &TripPlanningPage::addSelectedTeam);
    connect(ui->btnRemoveTeam, &QPushButton::clicked, this, &TripPlanningPage::removeSelectedTeam);
    connect(ui->btnMoveUp, &QPushButton::clicked, this, &TripPlanningPage::moveSelectedTeamUp);
    connect(ui->btnMoveDown, &QPushButton::clicked, this, &TripPlanningPage::moveSelectedTeamDown);
    connect(ui->btnGenerateTrip, &QPushButton::clicked, this, &TripPlanningPage::generateTrip);
    connect(ui->btnBackToMain, &QPushButton::clicked, this, &TripPlanningPage::close);
    connect(ui->btnContinueToTripDetail, &QPushButton::clicked,
            this, &TripPlanningPage::openTripDetailPage);

    connect(ui->cmbTripType, &QComboBox::currentIndexChanged, this, [this](int) {
        updateUiForTripType();
    });

    updateUiForTripType();
}

TripPlanningPage::~TripPlanningPage()
{
    delete ui;
}

void TripPlanningPage::loadStadiumAndTeamData()
{
    ui->cmbStartStadium->clear();
    ui->cmbTargetTeam->clear();
    ui->listAvailableTeams->clear();
    ui->listSelectedTeams->clear();

    auto stadiums = APP->stadiumRepository()->getAllStadiums(
        StadiumRepository::StadiumSortBy::TeamName,
        StadiumRepository::LeagueFilter::All
        );

    for (const auto& stadium : stadiums)
    {
        ui->cmbStartStadium->addItem(stadium.stadium_name, stadium.stadium_id);
        ui->cmbTargetTeam->addItem(stadium.team_name, stadium.stadium_id);
        ui->listAvailableTeams->addItem(stadium.team_name);
    }
}
void TripPlanningPage::addSelectedTeam()
{
    QListWidgetItem *currentItem = ui->listAvailableTeams->currentItem();
    if (!currentItem)
        return;

    QString teamName = currentItem->text();

    for (int i = 0; i < ui->listSelectedTeams->count(); ++i)
    {
        if (ui->listSelectedTeams->item(i)->text() == teamName)
            return;
    }

    ui->listSelectedTeams->addItem(teamName);
}

void TripPlanningPage::removeSelectedTeam()
{
    QListWidgetItem *currentItem = ui->listSelectedTeams->currentItem();
    if (!currentItem)
        return;

    delete ui->listSelectedTeams->takeItem(ui->listSelectedTeams->row(currentItem));
}

void TripPlanningPage::moveSelectedTeamUp()
{
    int row = ui->listSelectedTeams->currentRow();
    if (row <= 0)
        return;

    QListWidgetItem *item = ui->listSelectedTeams->takeItem(row);
    ui->listSelectedTeams->insertItem(row - 1, item);
    ui->listSelectedTeams->setCurrentRow(row - 1);
}

void TripPlanningPage::moveSelectedTeamDown()
{
    int row = ui->listSelectedTeams->currentRow();
    if (row < 0 || row >= ui->listSelectedTeams->count() - 1)
        return;

    QListWidgetItem *item = ui->listSelectedTeams->takeItem(row);
    ui->listSelectedTeams->insertItem(row + 1, item);
    ui->listSelectedTeams->setCurrentRow(row + 1);
}

int TripPlanningPage::getSelectedTargetTeamId() const
{
    return ui->cmbTargetTeam->currentData().toInt();
}

void TripPlanningPage::updateUiForTripType()
{
    QString tripType = ui->cmbTripType->currentText();

    bool showStartStadium = false;
    bool showTargetTeam = false;
    bool showTeamLists = false;
    bool allowReorder = false;

    QString description;

    if (tripType == "Shortest Trip from Dodger to Selected Team")
    {
        showTargetTeam = true;
        description = "Choose one target team. The trip starts at Dodger Stadium and follows the shortest path.";
    }
    else if (tripType == "Custom Ordered Trip")
    {
        showStartStadium = true;
        showTeamLists = true;
        allowReorder = true;
        description = "Choose a starting stadium and arrange the selected teams in the exact visit order.";
    }
    else if (tripType == "Visit All Teams from Marlins")
    {
        description = "Visits all stadiums starting at Marlins Park using the nearest-next strategy.";
    }
    else if (tripType == "Custom Efficient Trip")
    {
        showStartStadium = true;
        showTeamLists = true;
        description = "Choose a starting stadium and selected teams. The trip will visit them in the most efficient nearest-next order.";
    }
    else if (tripType == "Minimum Spanning Tree (MST)")
    {
        description = "Generates the minimum spanning tree connecting all MLB stadiums and displays total mileage.";
    }
    else if (tripType == "DFS from Oracle Park")
    {
        description = "Performs a DFS starting at Oracle Park. If there is a choice, the shortest distance should be preferred.";
    }
    else if (tripType == "BFS from Target Field")
    {
        description = "Performs a BFS starting at Target Field. If there is a choice, the shortest distance should be preferred.";
    }

    ui->lblStartStadium->setEnabled(showStartStadium);
    ui->cmbStartStadium->setEnabled(showStartStadium);

    ui->lblTargetTeam->setEnabled(showTargetTeam);
    ui->cmbTargetTeam->setEnabled(showTargetTeam);

    ui->lblAvailableTeams->setEnabled(showTeamLists);
    ui->listAvailableTeams->setEnabled(showTeamLists);

    ui->lblSelectedTeams->setEnabled(showTeamLists);
    ui->listSelectedTeams->setEnabled(showTeamLists);

    ui->btnAddTeam->setEnabled(showTeamLists);
    ui->btnRemoveTeam->setEnabled(showTeamLists);

    ui->btnMoveUp->setEnabled(showTeamLists && allowReorder);
    ui->btnMoveDown->setEnabled(showTeamLists && allowReorder);

    ui->lblModeDescription->setText(description);
}

std::vector<int> TripPlanningPage::getSelectedTeamIds() const
{
    std::vector<int> ids;

    for (int i = 0; i < ui->listSelectedTeams->count(); ++i)
    {
        QString teamName = ui->listSelectedTeams->item(i)->text();

        for (int j = 0; j < ui->cmbTargetTeam->count(); ++j)
        {
            if (ui->cmbTargetTeam->itemText(j) == teamName)
            {
                ids.push_back(ui->cmbTargetTeam->itemData(j).toInt());
                break;
            }
        }
    }

    return ids;
}

void TripPlanningPage::generateTrip()
{
    QString tripType = ui->cmbTripType->currentText();
    bool success = false;

    auto* planner = APP->tripPlanner();
    if (!planner)
    {
        QMessageBox::warning(this, "Trip Planner", "Trip planner is not available.");
        return;
    }

    int startId = ui->cmbStartStadium->currentData().toInt();
    int targetId = getSelectedTargetTeamId();
    std::vector<int> selectedIds = getSelectedTeamIds();

    // Remove the starting stadium if user accidentally added it
    selectedIds.erase(
        std::remove(selectedIds.begin(), selectedIds.end(), startId),
        selectedIds.end()
        );

    // Hard-coded IDs based on your database
    int dodgerId = 14;
    int marlinsId = 15;
    int oracleId = 24;
    int targetFieldId = 17;

    if (tripType == "Shortest Trip from Dodger to Selected Team")
    {
        if (targetId <= 0)
        {
            QMessageBox::warning(this, "Trip Planning", "Please select a target team.");
            return;
        }

        success = planner->planShortestTripToTarget(dodgerId, targetId);
    }
    else if (tripType == "Custom Ordered Trip")
    {
        if (selectedIds.empty())
        {
            QMessageBox::warning(this, "Trip Planning", "Please add at least one team to Selected Teams.");
            return;
        }

        success = planner->planCustomOrderedTrip(startId, selectedIds);
    }
    else if (tripType == "Visit All Teams from Marlins")
    {
        success = planner->planVisitAllByNearestFrom(marlinsId);
    }
    else if (tripType == "Custom Efficient Trip")
    {
        if (selectedIds.empty())
        {
            QMessageBox::warning(this, "Trip Planning", "Please choose at least one team other than the starting team.");
            return;
        }

        success = planner->planCustomUnorderedEfficientTrip(startId, selectedIds);
    }
    else if (tripType == "Minimum Spanning Tree (MST)")
    {
        success = planner->generateMSTResult();
    }
    else if (tripType == "DFS from Oracle Park")
    {
        success = planner->generateDFSResultFrom(oracleId);
    }
    else if (tripType == "BFS from Target Field")
    {
        success = planner->generateBFSResultFrom(targetFieldId);
    }

    if (success)
    {
        ui->btnContinueToTripDetail->setEnabled(true);
        QMessageBox::information(this, "Trip Planning", "Trip generated successfully.");
    }
    else
    {
        ui->btnContinueToTripDetail->setEnabled(false);
        QMessageBox::warning(this, "Trip Planning", "Failed to generate trip.");
    }
}

void TripPlanningPage::openTripDetailPage()
{
    auto* planner = APP->tripPlanner();
    if (!planner || planner->getCurrentTrip() == nullptr)
    {
        QMessageBox::warning(this, "Trip Planning",
                             "Please generate a trip before opening Trip Detail.");
        return;
    }

    TripDetailPage *tripDetailPage = new TripDetailPage();
    tripDetailPage->setAttribute(Qt::WA_DeleteOnClose);
    tripDetailPage->show();
}
