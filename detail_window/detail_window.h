#ifndef DETAIL_WINDOW_H
#define DETAIL_WINDOW_H

#include <QMainWindow>
#include "../data_types.h"

namespace Ui {
class detail_window;
}

class DetailWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DetailWindow(QWidget *parent = nullptr);
    explicit DetailWindow(const Stadium& stadium, QWidget *parent = nullptr);
    ~DetailWindow();

    void setStadium(const Stadium& stadium);

private:
    Ui::detail_window *ui;
};

#endif // DETAIL_WINDOW_H
