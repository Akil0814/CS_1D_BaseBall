#include "browse_window.h"
#include "ui_browse_window.h"
#include "../detail_window/detail_window.h"
#include "../App/application.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidgetItem>

BrowseWindow::BrowseWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::browse_window)
{
    ui->setupUi(this);

    ui->tblBrowse->setStyleSheet(
        "QTableWidget::item:selected {"
        "background-color: rgb(220, 235, 250);"
        "color: black;"
        "}"
        );

    setupTable();
    loadBrowseTable();

    connect(ui->btnBackToMain, &QPushButton::clicked, this, [this]() {
        this->close();
    });

    connect(ui->btnMoreDetail, &QPushButton::clicked, this, [this]() {
        openDetailWindowForSelectedRow();
    });

    connect(ui->tblBrowse, &QTableWidget::cellDoubleClicked, this,
            [this](int, int) {
                openDetailWindowForSelectedRow();
            });

    connect(ui->cmbLeague, &QComboBox::currentIndexChanged, this,
            [this](int) {
                loadBrowseTable();
            });

    connect(ui->cmbSortBy, &QComboBox::currentIndexChanged, this,
            [this](int) {
                loadBrowseTable();
            });

    connect(ui->cmbBrowseBy, &QComboBox::currentIndexChanged, this,
            [this](int) {
                loadBrowseTable();
            });
}

BrowseWindow::~BrowseWindow()
{
    delete ui;
}

void BrowseWindow::setupTable()
{
    ui->tblBrowse->setColumnCount(4);

    QStringList headers;
    headers << "Team Name" << "Stadium Name" << "League" << "Location";
    ui->tblBrowse->setHorizontalHeaderLabels(headers);

    ui->tblBrowse->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblBrowse->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblBrowse->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblBrowse->setAlternatingRowColors(true);
    ui->tblBrowse->horizontalHeader()->setStretchLastSection(true);
    ui->tblBrowse->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tblBrowse->verticalHeader()->setVisible(false);
}

StadiumRepository::StadiumSortBy BrowseWindow::getSelectedSort() const
{
    QString sortText = ui->cmbSortBy->currentText();

    if (sortText == "Team Name")
        return StadiumRepository::StadiumSortBy::TeamName;
    if (sortText == "Stadium Name")
        return StadiumRepository::StadiumSortBy::StadiumName;
    if (sortText == "League")
        return StadiumRepository::StadiumSortBy::League;
    if (sortText == "Date Opened")
        return StadiumRepository::StadiumSortBy::DateOpened;
    if (sortText == "Seat Capacity")
        return StadiumRepository::StadiumSortBy::SeatingCapacity;
    if (sortText == "Distance to Center Field")
        return StadiumRepository::StadiumSortBy::DistanceToCenterField;
    if (sortText == "Ballpark Typology")
        return StadiumRepository::StadiumSortBy::Typology;
    if (sortText == "Open Roof")
        return StadiumRepository::StadiumSortBy::OpenRoof;

    return StadiumRepository::StadiumSortBy::TeamName;
}

StadiumRepository::LeagueFilter BrowseWindow::getSelectedLeague() const
{
    QString leagueText = ui->cmbLeague->currentText();

    if (leagueText == "American League")
        return StadiumRepository::LeagueFilter::American;
    if (leagueText == "National League")
        return StadiumRepository::LeagueFilter::National;

    return StadiumRepository::LeagueFilter::All;
}

bool BrowseWindow::isBrowsingByStadium() const
{
    return ui->cmbBrowseBy->currentText() == "Stadium";
}

void BrowseWindow::loadBrowseTable()
{
    ui->tblBrowse->setRowCount(0);
    stadiums.clear();

    if (APP == nullptr || APP->stadiumRepository() == nullptr)
    {
        QMessageBox::warning(this, "Database Error", "Stadium repository is not available.");
        return;
    }

    stadiums = APP->stadiumRepository()->getAllStadiums(
        getSelectedSort(),
        getSelectedLeague(),
        isBrowsingByStadium()
        );

    QStringList headers;
    if (ui->cmbBrowseBy->currentText() == "Stadium")
        headers << "Stadium Name" << "Team Name" << "League" << "Location";
    else
        headers << "Team Name" << "Stadium Name" << "League" << "Location";

    ui->tblBrowse->setHorizontalHeaderLabels(headers);
    ui->tblBrowse->setRowCount(static_cast<int>(stadiums.size()));

    for (int row = 0; row < static_cast<int>(stadiums.size()); ++row)
    {
        const Stadium& s = stadiums[row];

        QString firstColumnText;
        QString secondColumnText;

        if (ui->cmbBrowseBy->currentText() == "Stadium")
        {
            firstColumnText = s.stadium_name;
            secondColumnText = s.team_name;
        }
        else
        {
            firstColumnText = s.team_name;
            secondColumnText = s.stadium_name;
        }

        ui->tblBrowse->setItem(row, 0, new QTableWidgetItem(firstColumnText));
        ui->tblBrowse->setItem(row, 1, new QTableWidgetItem(secondColumnText));
        ui->tblBrowse->setItem(row, 2, new QTableWidgetItem(s.league));
        ui->tblBrowse->setItem(row, 3, new QTableWidgetItem(s.location));
    }

    if (!stadiums.empty())
        ui->tblBrowse->selectRow(0);
}

void BrowseWindow::openDetailWindowForSelectedRow()
{
    int row = ui->tblBrowse->currentRow();

    if (row < 0 || row >= static_cast<int>(stadiums.size()))
    {
        QMessageBox::information(this, "Selection Required", "Please select a stadium first.");
        return;
    }

    DetailWindow *detailWindow = new DetailWindow(stadiums[row], this);
    detailWindow->show();
}
