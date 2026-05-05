#ifndef TRIP_DETAIL_PAGE_H
#define TRIP_DETAIL_PAGE_H

#include "../data_types.h"
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
    void setCurrentStadium(const Stadium& stadium);

private:
    void openMoreInfo();
    void selectPreviousStop();
    void selectNextStop();
    void updateNavigationButtons();
    void updateStadiumSummary();

    Ui::TripDetailPage *ui;
    Stadium currentStadium;
    bool hasCurrentStadium = false;
};

#endif // TRIP_DETAIL_PAGE_H
