#ifndef BROWSE_WINDOW_H
#define BROWSE_WINDOW_H

#include <QMainWindow>
#include <vector>
#include "../data_types.h"
#include "../App/data_access/stadium_repository.h"

namespace Ui {
class browse_window;
}

class BrowseWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BrowseWindow(QWidget *parent = nullptr);
    ~BrowseWindow();

private:
    void loadBrowseTable();
    void setupTableForTeamView();
    void setupTableForStadiumView();
    StadiumRepository::LeagueFilter currentLeagueFilter() const;
    StadiumRepository::StadiumSortBy currentSortOption() const;
    Stadium selectedStadiumFromCurrentRow() const;

private:
    Ui::browse_window *ui;
    std::vector<Stadium> _currentStadiums;
};

#endif // BROWSE_WINDOW_H
