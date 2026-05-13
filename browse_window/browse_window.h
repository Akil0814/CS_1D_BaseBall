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
    enum class CenterFieldFilter
    {
        All,
        MaximumOnly,
        MinimumOnly
    };

    void loadBrowseTable();
    void setupTable();
    StadiumRepository::StadiumSortBy getSelectedSort() const;
    StadiumRepository::LeagueFilter getSelectedLeague() const;
    CenterFieldFilter getSelectedCenterFieldFilter() const;
    bool isBrowsingByStadium() const;
    void openDetailWindowForSelectedRow();
    void applyCenterFieldFilter(std::vector<Stadium>& stadiumList) const;

    bool isSeatCapacitySort() const;
    void setHeaders();
    QString stadiumValueForColumn(const Stadium& s, int dataColumnIndex) const;

private:
    Ui::browse_window *ui;
    std::vector<Stadium> stadiums;
};

#endif // BROWSE_WINDOW_H
