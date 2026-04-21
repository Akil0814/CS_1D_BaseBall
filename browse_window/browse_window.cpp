#include "browse_window.h"
#include "ui_browse_window.h"
#include "detail_window.h"
#include "../App/application.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidgetItem>

BrowseWindow::BrowseWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::browse_window)
{
    ui->setupUi(this);

    ui->btnMoreDetail->setEnabled(false);

    ui->tblBrowse->setColumnCount(3);
    ui->tblBrowse->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblBrowse->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblBrowse->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblBrowse->horizontalHeader()->setStretchLastSection(true);
    ui->tblBrowse->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tblBrowse->verticalHeader()->setVisible(false);

    connect(ui->btnBackToMain, &QPushButton::clicked, this, [this]() {
        this->close();
    });

    connect(ui->cmbLeague, &QComboBox::currentTextChanged, this, [this]() {
        loadBrowseTable();
    });

    connect(ui->cmbBrowseBy, &QComboBox::currentTextChanged, this, [this]() {
        loadBrowseTable();
    });

    connect(ui->cmbSortBy, &QComboBox::currentTextChanged, this, [this]() {
        loadBrowseTable();
    });

    connect(ui->tblBrowse, &QTableWidget::itemSelectionChanged, this, [this]() {
        ui->btnMoreDetail->setEnabled(ui->tblBrowse->currentRow() >= 0);
    });

    connect(ui->btnMoreDetail, &QPushButton::clicked, this, [this]() {
        if (ui->tblBrowse->currentRow() < 0)
        {
            QMessageBox::information(this, "No Selection", "Please select a row first.");
            return;
        }

        Stadium stadium = selectedStadiumFromCurrentRow();
        DetailWindow *detailWindow = new DetailWindow(stadium, this);
        detailWindow->show();
    });

    loadBrowseTable();
}

BrowseWindow::~BrowseWindow()
{
    delete ui;
}

StadiumRepository::LeagueFilter BrowseWindow::currentLeagueFilter() const
{
    QString league = ui->cmbLeague->currentText();

    if (league == "American League")
        return StadiumRepository::LeagueFilter::American;
    if (league == "National League")
        return StadiumRepository::LeagueFilter::National;

    return StadiumRepository::LeagueFilter::All;
}

StadiumRepository::StadiumSortBy BrowseWindow::currentSortOption() const
{
    QString sortText = ui->cmbSortBy->currentText();

    if (sortText == "Stadium Name")
        return StadiumRepository::StadiumSortBy::StadiumName;
    if (sortText == "Date Opened")
        return StadiumRepository::StadiumSortBy::DateOpened;
    if (sortText == "Seating Capacity")
        return StadiumRepository::StadiumSortBy::SeatingCapacity;
    if (sortText == "Ballpark Typology")
        return StadiumRepository::StadiumSortBy::Typology;

    return StadiumRepository::StadiumSortBy::TeamName;
}

void BrowseWindow::setupTableForTeamView()
{
    ui->tblBrowse->setHorizontalHeaderLabels(QStringList() << "Team" << "Stadium" << "League");
}

void BrowseWindow::setupTableForStadiumView()
{
    ui->tblBrowse->setHorizontalHeaderLabels(QStringList() << "Stadium" << "Team" << "Location");
}

void BrowseWindow::loadBrowseTable()
{
    auto *repo = APP->stadiumRepository();
    if (!repo)
    {
        QMessageBox::critical(this, "Database Error", "Stadium repository is not available.");
        return;
    }

    _currentStadiums = repo->getAllStadiums(currentSortOption(), currentLeagueFilter());

    ui->tblBrowse->clearContents();
    ui->tblBrowse->setRowCount(static_cast<int>(_currentStadiums.size()));

    bool browseByTeam = (ui->cmbBrowseBy->currentText() == "Team");

    if (browseByTeam)
        setupTableForTeamView();
    else
        setupTableForStadiumView();

    for (int row = 0; row < static_cast<int>(_currentStadiums.size()); ++row)
    {
        const Stadium& stadium = _currentStadiums[row];

        if (browseByTeam)
        {
            ui->tblBrowse->setItem(row, 0, new QTableWidgetItem(stadium.team_name));
            ui->tblBrowse->setItem(row, 1, new QTableWidgetItem(stadium.stadium_name));
            ui->tblBrowse->setItem(row, 2, new QTableWidgetItem(stadium.league));
        }
        else
        {
            ui->tblBrowse->setItem(row, 0, new QTableWidgetItem(stadium.stadium_name));
            ui->tblBrowse->setItem(row, 1, new QTableWidgetItem(stadium.team_name));
            ui->tblBrowse->setItem(row, 2, new QTableWidgetItem(stadium.location));
        }
    }

    ui->btnMoreDetail->setEnabled(false);
}

Stadium BrowseWindow::selectedStadiumFromCurrentRow() const
{
    int row = ui->tblBrowse->currentRow();

    if (row >= 0 && row < static_cast<int>(_currentStadiums.size()))
        return _currentStadiums[row];

    return Stadium{};
}
