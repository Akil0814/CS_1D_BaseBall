#ifndef TRIP_DETAIL_PAGE_H
#define TRIP_DETAIL_PAGE_H

#include "../data_types.h"
#include "../App/model/trip.h"
#include <QWidget>

namespace Ui {
class TripDetailPage;
}

class TripDetailPage : public QWidget
{
    Q_OBJECT

public:
    explicit TripDetailPage(QWidget *parent = nullptr);
    ~TripDetailPage();

private slots:
    void on_btnViewCart_clicked();
    void on_btnEndTrip_clicked();
    void on_btnPreviousStop_clicked();
    void on_btnNextStop_clicked();
    void on_btnMoreInfo_clicked();
    void on_btnAddToCart_clicked();
    void on_lstTripStops_currentRowChanged(int current_row);

private:
    void loadTripStops();
    void openMoreInfo();
    void selectPreviousStop();
    void selectNextStop();
    void loadStadiumImage();
    void updateNavigationButtons();
    void updateTripStopStyles();
    void updateStadiumSummary();

    Ui::TripDetailPage *_ui;
    Trip* _current_trip = nullptr;
    const std::vector<Stadium>* _all_stadiums = nullptr;
    const Stadium* _current_stadium = nullptr;
    bool _has_current_stadium = false;
};

#endif // TRIP_DETAIL_PAGE_H
