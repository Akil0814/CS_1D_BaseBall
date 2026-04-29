#ifndef TOUR_SHOP_WINDOW_H
#define TOUR_SHOP_WINDOW_H

#include <QMainWindow>

namespace Ui {
class tour_shop_window;
}

class TourShopWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TourShopWindow(QWidget *parent = nullptr);
    ~TourShopWindow();

private:
    Ui::tour_shop_window *ui;
};

#endif // TOUR_SHOP_WINDOW_H
