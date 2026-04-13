#ifndef DETAIL_WINDOW_H
#define DETAIL_WINDOW_H

#include <QMainWindow>

namespace Ui {
class detail_window;
}

class DetailWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DetailWindow(QWidget *parent = nullptr);
    ~DetailWindow();

private:
    Ui::detail_window *ui;
};

#endif // DETAIL_WINDOW_H
