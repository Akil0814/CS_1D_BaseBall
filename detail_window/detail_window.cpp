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

DetailWindow::DetailWindow(const Stadium& stadium, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::detail_window)
{
    ui->setupUi(this);

    connect(ui->btnClose, &QPushButton::clicked, this, &DetailWindow::close);

    setStadium(stadium);
}

void DetailWindow::setStadium(const Stadium& stadium)
{
    ui->lblTeamNameValue->setText(stadium.team_name);
    ui->lblStadiumNameValue->setText(stadium.stadium_name);
    ui->lblLeagueValue->setText(stadium.league);
    ui->lblLocationValue->setText(stadium.location);
    ui->lblPlayingSurfaceValue->setText(stadium.playing_surface);
    ui->lblRoofTypeValue->setText(stadium.roof_type);
    ui->lblDateOpenedValue->setText(QString::number(stadium.date_opened));
    ui->lblSeatingCapacityValue->setText(QString::number(stadium.seating_capacity));
    ui->lblDistanceCenterFieldValue->setText(QString::number(stadium.distance_to_center_field_ft) + " ft");
    ui->lblBallparkTypologyValue->setText(stadium.ballpark_typology);
}

DetailWindow::~DetailWindow()
{
    delete ui;
}
