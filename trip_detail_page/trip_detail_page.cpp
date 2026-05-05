#include "trip_detail_page.h"
#include "ui_trip_detail_page.h"
#include "../detail_window/detail_window.h"
#include "../App/application.h"

#include <QFile>
#include <QMessageBox>
#include <QPixmap>

TripDetailPage::TripDetailPage(QWidget *parent)
    : QWidget(parent)
    , _ui(new Ui::TripDetailPage)
{
    _ui->setupUi(this);

    _current_trip = APP->getTripPlanner()->getCurrentTrip();

    if (!_current_trip)
    {
        updateStadiumSummary();
        updateNavigationButtons();
        return;
    }

    _all_stadiums = &(_current_trip->getResult().stadiums);
    loadTripStops();

    _current_stadium = _current_trip->getCurrentStop();
    _has_current_stadium = (_current_stadium != nullptr);

    if (_current_trip->stopCount() > 0)
        _ui->lstTripStops->setCurrentRow(static_cast<int>(_current_trip->currentStopIndex()));

    updateStadiumSummary();
    updateNavigationButtons();
}

TripDetailPage::~TripDetailPage()
{
    delete _ui;
}

void TripDetailPage::on_btnViewCart_clicked()
{
}

void TripDetailPage::on_btnEndTrip_clicked()
{
}

void TripDetailPage::on_btnPreviousStop_clicked()
{
    selectPreviousStop();
}

void TripDetailPage::on_btnNextStop_clicked()
{
    selectNextStop();
}

void TripDetailPage::on_btnMoreInfo_clicked()
{
    openMoreInfo();
}

void TripDetailPage::on_btnAddToCart_clicked()
{

}

void TripDetailPage::on_lstTripStops_currentRowChanged(int current_row)
{
    if (_all_stadiums == nullptr ||
        current_row < 0 ||
        current_row >= static_cast<int>(_all_stadiums->size()))
    {
        _current_stadium = nullptr;
        _has_current_stadium = false;
        _ui->lblTripProgress->setText(
            QString("Stop 0 of %1").arg(_all_stadiums == nullptr ? 0 : static_cast<int>(_all_stadiums->size()))
            );
        updateStadiumSummary();
        updateNavigationButtons();
        return;
    }

    _current_stadium = &(*_all_stadiums)[current_row];
    _has_current_stadium = true;
    _ui->lblTripProgress->setText(
        QString("Stop %1 of %2").arg(current_row + 1).arg(static_cast<int>(_all_stadiums->size()))
        );
    updateStadiumSummary();
    updateNavigationButtons();
}

void TripDetailPage::loadTripStops()
{
    _ui->lstTripStops->clear();

    if (_all_stadiums == nullptr)
        return;

    for (int index = 0; index < static_cast<int>(_all_stadiums->size()); ++index)
    {
        const Stadium& stadium = (*_all_stadiums)[index];
        const QString display_text = QString("%1. %2")
                                         .arg(index + 1)
                                         .arg(stadium.stadium_name);
        _ui->lstTripStops->addItem(display_text);
    }
}


void TripDetailPage::openMoreInfo()
{
    if (!_has_current_stadium)
    {
        QMessageBox::information(this, "No Stadium Selected",
                                 "Select a stadium before opening more information.");
        return;
    }

    DetailWindow *detail_window = new DetailWindow(*_current_stadium, this);
    detail_window->setAttribute(Qt::WA_DeleteOnClose);
    detail_window->show();
}

void TripDetailPage::selectPreviousStop()
{
    const int current_row = _ui->lstTripStops->currentRow();
    if (current_row > 0)
        _ui->lstTripStops->setCurrentRow(current_row - 1);
}

void TripDetailPage::selectNextStop()
{
    const int current_row = _ui->lstTripStops->currentRow();
    const int last_row = _ui->lstTripStops->count() - 1;

    if (current_row >= 0 && current_row < last_row)
        _ui->lstTripStops->setCurrentRow(current_row + 1);
}

void TripDetailPage::updateNavigationButtons()
{
    const int count = _ui->lstTripStops->count();
    const int current_row = _ui->lstTripStops->currentRow();
    const bool has_selection = current_row >= 0 && current_row < count;

    _ui->btnPreviousStop->setEnabled(has_selection && current_row > 0);
    _ui->btnNextStop->setEnabled(has_selection && current_row < count - 1);
}

void TripDetailPage::loadStadiumImage()
{
    if (!_has_current_stadium || APP == nullptr)
    {
        _ui->lblStadiumImage->setPixmap(QPixmap());
        _ui->lblStadiumImage->setText("No image available");
        _ui->lblStadiumImage->setAlignment(Qt::AlignCenter);
        return;
    }

    QString assets_path = APP->assetsDir() + "/images/stadiums/";
    QString image_path = assets_path + QString::number(_current_stadium->stadium_id) + ".jpg";

    QPixmap pixmap;

    if (QFile::exists(image_path) && pixmap.load(image_path))
    {
        _ui->lblStadiumImage->setPixmap(
            pixmap.scaled(_ui->lblStadiumImage->size(),
                          Qt::KeepAspectRatio,
                          Qt::SmoothTransformation)
            );
        _ui->lblStadiumImage->setAlignment(Qt::AlignCenter);
        _ui->lblStadiumImage->setText("");
    }
    else
    {
        _ui->lblStadiumImage->setPixmap(QPixmap());
        _ui->lblStadiumImage->setText("No image available");
        _ui->lblStadiumImage->setAlignment(Qt::AlignCenter);
    }
}

void TripDetailPage::updateStadiumSummary()
{
    if (!_has_current_stadium)
    {
        _ui->lblSelectedStadium->setText("No stadium selected");
        _ui->lblTeamNameValue->setText("N/A");
        _ui->lblLeagueValue->setText("N/A");
        _ui->lblLocationValue->setText("N/A");
        _ui->btnMoreInfo->setEnabled(false);
        loadStadiumImage();
        return;
    }

    _ui->lblSelectedStadium->setText(_current_stadium->stadium_name);
    _ui->lblTeamNameValue->setText(_current_stadium->team_name);
    _ui->lblLeagueValue->setText(_current_stadium->league);
    _ui->lblLocationValue->setText(_current_stadium->location);
    _ui->btnMoreInfo->setEnabled(true);
    loadStadiumImage();
}
