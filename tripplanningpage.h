#ifndef TRIPPLANNINGPAGE_H
#define TRIPPLANNINGPAGE_H

#include <QMainWindow>

namespace Ui {
class TripPlanningPage;
}

class TripPlanningPage : public QMainWindow
{
    Q_OBJECT

public:
    explicit TripPlanningPage(QWidget *parent = nullptr);
    ~TripPlanningPage();

private:
    void updateUiForTripType();
    void loadStadiumAndTeamData();
    void addSelectedTeam();
    void removeSelectedTeam();
    void moveSelectedTeamUp();
    void moveSelectedTeamDown();
    void generateTrip();
    int getSelectedTargetTeamId() const;
    std::vector<int> getSelectedTeamIds() const;
    void openTripDetailPage();

private:
    Ui::TripPlanningPage *ui;
};

#endif // TRIPPLANNINGPAGE_H
