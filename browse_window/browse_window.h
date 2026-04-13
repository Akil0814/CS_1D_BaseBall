#ifndef BROWSE_WINDOW_H
#define BROWSE_WINDOW_H

#include <QMainWindow>

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
    Ui::browse_window *ui;
};

#endif // BROWSE_WINDOW_H
