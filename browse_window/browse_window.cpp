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
        close();
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

    connect(ui->cmbCenterFieldFilter, &QComboBox::currentIndexChanged, this,
            [this](int) {
                loadBrowseTable();
            });
}

BrowseWindow::~BrowseWindow()
{
    delete ui;
}

bool BrowseWindow::isSeatCapacitySort() const
{
    return ui->cmbSortBy->currentText() == "Seat Capacity";
}

void BrowseWindow::setHeaders()
{
    QStringList headers;
    headers << "#"
            << "ID";

    if (isBrowsingByStadium())
        headers << "Stadium Name" << "Team Name";
    else
        headers << "Team Name" << "Stadium Name";

    headers << "League"
            << "Location"
            << "Seat Capacity"
            << "Playing Surface"
            << "Date Opened"
            << "Distance to Center Field (ft)"
            << "Distance to Center Field"
            << "Ballpark Typology"
            << "Roof Type";

    ui->tblBrowse->setColumnCount(headers.size());
    ui->tblBrowse->setHorizontalHeaderLabels(headers);
}

QString BrowseWindow::stadiumValueForColumn(const Stadium& s, int dataColumnIndex) const
{
    switch (dataColumnIndex)
    {
    case 0:
        return QString::number(s.stadium_id);

    case 1:
        return isBrowsingByStadium() ? s.stadium_name : s.team_name;

    case 2:
        return isBrowsingByStadium() ? s.team_name : s.stadium_name;

    case 3:
        return s.league;

    case 4:
        return s.location;

    case 5:
        return QString::number(s.seating_capacity);

    case 6:
        return s.playing_surface;

    case 7:
        return QString::number(s.date_opened);

    case 8:
        return QString::number(s.distance_to_center_field_ft);

    case 9:
        return s.distance_to_center_field_raw;

    case 10:
        return s.ballpark_typology;

    case 11:
        return s.roof_type;

    default:
        return "";
    }
}

void BrowseWindow::setupTable()
{
    setHeaders();

    ui->tblBrowse->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblBrowse->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblBrowse->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblBrowse->setAlternatingRowColors(true);

    ui->tblBrowse->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tblBrowse->horizontalHeader()->setStretchLastSection(false);
    ui->tblBrowse->verticalHeader()->setVisible(false);

    ui->tblBrowse->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tblBrowse->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
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

BrowseWindow::CenterFieldFilter BrowseWindow::getSelectedCenterFieldFilter() const
{
    const QString filterText = ui->cmbCenterFieldFilter->currentText();

    if (filterText == "Only Max Center Field")
        return CenterFieldFilter::MaximumOnly;
    if (filterText == "Only Min Center Field")
        return CenterFieldFilter::MinimumOnly;

    return CenterFieldFilter::All;
}

bool BrowseWindow::isBrowsingByStadium() const
{
    return ui->cmbBrowseBy->currentText() == "Stadium";
}

void BrowseWindow::applyCenterFieldFilter(std::vector<Stadium>& stadiumList) const
{
    if (stadiumList.empty())
        return;

    const CenterFieldFilter filter = getSelectedCenterFieldFilter();
    if (filter == CenterFieldFilter::All)
        return;

    int targetDistance = stadiumList.front().distance_to_center_field_ft;
    for (const Stadium& stadium : stadiumList)
    {
        if (filter == CenterFieldFilter::MaximumOnly
            && stadium.distance_to_center_field_ft > targetDistance)
            targetDistance = stadium.distance_to_center_field_ft;
        else if (filter == CenterFieldFilter::MinimumOnly
                 && stadium.distance_to_center_field_ft < targetDistance)
            targetDistance = stadium.distance_to_center_field_ft;
    }

    std::vector<Stadium> filteredStadiums;
    for (const Stadium& stadium : stadiumList)
    {
        if (stadium.distance_to_center_field_ft == targetDistance)
            filteredStadiums.push_back(stadium);
    }

    stadiumList = std::move(filteredStadiums);
}

void BrowseWindow::loadBrowseTable()
{
    ui->tblBrowse->clearContents();
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
    applyCenterFieldFilter(stadiums);

    setHeaders();

    int totalSeatCapacity = 0;
    for (const Stadium& s : stadiums)
        totalSeatCapacity += s.seating_capacity;

    int rowCount = static_cast<int>(stadiums.size());
    if (isSeatCapacitySort())
        rowCount += 1;

    ui->tblBrowse->setRowCount(rowCount);

    for (int row = 0; row < static_cast<int>(stadiums.size()); ++row)
    {
        const Stadium& s = stadiums[row];

        QTableWidgetItem *rowNumberItem = new QTableWidgetItem(QString::number(row + 1));
        rowNumberItem->setTextAlignment(Qt::AlignCenter);
        ui->tblBrowse->setItem(row, 0, rowNumberItem);

        for (int dataCol = 0; dataCol <= 11; ++dataCol)
        {
            QTableWidgetItem *item =
                new QTableWidgetItem(stadiumValueForColumn(s, dataCol));
            item->setTextAlignment(Qt::AlignCenter);
            ui->tblBrowse->setItem(row, dataCol + 1, item);
        }
    }

    if (isSeatCapacitySort())
    {
        int totalRow = static_cast<int>(stadiums.size());

        for (int col = 0; col < ui->tblBrowse->columnCount(); ++col)
        {
            QTableWidgetItem *item = new QTableWidgetItem("");
            item->setTextAlignment(Qt::AlignCenter);
            ui->tblBrowse->setItem(totalRow, col, item);
        }

        ui->tblBrowse->item(totalRow, 6)->setText(QString::number(totalSeatCapacity));
    }

    if (!stadiums.empty())
        ui->tblBrowse->selectRow(0);
}

void BrowseWindow::openDetailWindowForSelectedRow()
{
    int row = ui->tblBrowse->currentRow();

    // Ignore invalid row and ignore the final total row
    if (row < 0 || row >= static_cast<int>(stadiums.size()))
    {
        QMessageBox::information(this, "Selection Required", "Please select a stadium first.");
        return;
    }

    DetailWindow *detailWindow = new DetailWindow(stadiums[row], this);
    detailWindow->show();
}
