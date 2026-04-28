#include "detail_window.h"
#include "ui_detail_window.h"
#include "../App/application.h"

#include <QHeaderView>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QPixmap>
#include <QFile>

DetailWindow::DetailWindow(const Stadium& stadium, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::detail_window)
    , currentStadium(stadium)
{
    ui->setupUi(this);

    loadStadiumDetails();
    loadSouvenirs();
    loadStadiumImage();

    connect(ui->btnClose, &QPushButton::clicked, this, &DetailWindow::close);
}

DetailWindow::~DetailWindow()
{
    delete ui;
}

void DetailWindow::loadStadiumDetails()
{
    ui->lblDetailTitle->setText(currentStadium.stadium_name);

    ui->lblTeamNameValue->setText(currentStadium.team_name);
    ui->lblStadiumNameValue->setText(currentStadium.stadium_name);
    ui->lblLeagueValue->setText(currentStadium.league);
    ui->lblLocationValue->setText(currentStadium.location);
    ui->lblPlayingSurfaceValue->setText(currentStadium.playing_surface);
    ui->lblRoofTypeValue->setText(currentStadium.roof_type);
    ui->lblDateOpenedValue->setText(QString::number(currentStadium.date_opened));
    ui->lblSeatingCapacityValue->setText(QString::number(currentStadium.seating_capacity));

    if (!currentStadium.distance_to_center_field_raw.trimmed().isEmpty())
        ui->lblDistanceCenterFieldValue->setText(currentStadium.distance_to_center_field_raw);
    else
        ui->lblDistanceCenterFieldValue->setText(QString::number(currentStadium.distance_to_center_field_ft) + " ft");

    ui->lblBallparkTypologyValue->setText(currentStadium.ballpark_typology);
}

void DetailWindow::loadSouvenirs()
{
    ui->tblSouvenirs->clearContents();
    ui->tblSouvenirs->setRowCount(0);
    ui->tblSouvenirs->setColumnCount(2);

    QStringList headers;
    headers << "Souvenir" << "Price";
    ui->tblSouvenirs->setHorizontalHeaderLabels(headers);

    ui->tblSouvenirs->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblSouvenirs->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblSouvenirs->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblSouvenirs->setAlternatingRowColors(true);
    ui->tblSouvenirs->verticalHeader()->setVisible(false);
    ui->tblSouvenirs->horizontalHeader()->setStretchLastSection(true);
    ui->tblSouvenirs->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    if (APP == nullptr || APP->souvenirRepository() == nullptr)
    {

        return;
    }



    std::vector<Souvenir> souvenirs =
        APP->souvenirRepository()->getSouvenirsByStadiumID(currentStadium.stadium_id);



    ui->tblSouvenirs->setRowCount(static_cast<int>(souvenirs.size()));

    for (int row = 0; row < static_cast<int>(souvenirs.size()); ++row)
    {
        const Souvenir& s = souvenirs[row];
        ui->tblSouvenirs->setItem(row, 0, new QTableWidgetItem(s.name));
        ui->tblSouvenirs->setItem(row, 1, new QTableWidgetItem("$" + QString::number(s.price, 'f', 2)));
    }
}
void DetailWindow::loadStadiumImage()
{
    QString assetsPath = APP->assetsDir()+ "/images/stadiums/";
    QString imagePath = assetsPath + QString::number(currentStadium.stadium_id) + ".jpg";

    QPixmap pix;

    if (QFile::exists(imagePath) && pix.load(imagePath))
    {
        ui->lblStadiumImage->setPixmap(
            pix.scaled(ui->lblStadiumImage->size(),
                       Qt::KeepAspectRatio,
                       Qt::SmoothTransformation)
            );
        ui->lblStadiumImage->setAlignment(Qt::AlignCenter);
        ui->lblStadiumImage->setText("");
    }
    else
    {
        ui->lblStadiumImage->setText("No image available");
        ui->lblStadiumImage->setAlignment(Qt::AlignCenter);
    }
}
