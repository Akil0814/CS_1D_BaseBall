#ifndef DETAIL_WINDOW_H
#define DETAIL_WINDOW_H

#include <QMainWindow>
#include <vector>
#include "../data_types.h"

namespace Ui {
class detail_window;
}

class DetailWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DetailWindow(const Stadium& stadium, QWidget *parent = nullptr);
    ~DetailWindow();

private:
    void loadStadiumDetails();
    void loadSouvenirs();
    void loadStadiumImage();

private:
    Ui::detail_window *ui;
    Stadium currentStadium;
};

#endif // DETAIL_WINDOW_H
