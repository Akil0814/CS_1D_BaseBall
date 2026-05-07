#ifndef TRIP_DETAIL_PAGE_H
#define TRIP_DETAIL_PAGE_H

#include "../data_types.h"
#include "../App/model/trip.h"
#include <QString>
#include <QWidget>

namespace Ui {
class TripDetailPage;
}

class QLabel;
class QResizeEvent;

class TripDetailPage : public QWidget
{
    Q_OBJECT

public:
    explicit TripDetailPage(QWidget *parent = nullptr);
    ~TripDetailPage();

private slots:
    void handleViewCartClick();
    void handleEndTripClick();
    void handlePreviousStopClick();
    void handleNextStopClick();
    void handleMoreInfoClick();
    void handleAddToCartClick();
    void handleTripStopsCurrentRowChange(int currentRow);

private:
    void loadTripStops();
    void loadSouvenirs();
    void openMoreInfo();
    void syncFromCurrentTrip();
    void selectPreviousStop();
    void selectNextStop();
    void loadStadiumImage();
    void setElidedLabelText(QLabel *label, const QString& full_text);
    void updateDisplayedStadiumTexts();
    void updateNavigationButtons();
    void updateTripStopStyles();
    void updateStadiumSummary();

protected:
    void resizeEvent(QResizeEvent *event) override;

    Ui::TripDetailPage *_ui;
    Trip* _current_trip = nullptr;
    const std::vector<Stadium>* _all_stadiums = nullptr;
    const Stadium* _current_stadium = nullptr;
    bool _has_current_stadium = false;
    bool _is_syncing_selection = false;
    QString _selected_stadium_text;
    QString _team_name_text;
    QString _league_text;
    QString _location_text;
};

#endif // TRIP_DETAIL_PAGE_H
