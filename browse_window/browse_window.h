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
    void setupTable();
    StadiumRepository::StadiumSortBy getSelectedSort() const;
    StadiumRepository::LeagueFilter getSelectedLeague() const;
    bool isBrowsingByStadium() const;
    void openDetailWindowForSelectedRow();

private:
    Ui::browse_window *ui;
    std::vector<Stadium> stadiums;
};

#endif // BROWSE_WINDOW_H
