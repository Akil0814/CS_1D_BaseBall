#ifndef MAP_WINDOW_H
#define MAP_WINDOW_H

#include <QMainWindow>

namespace Ui {
class MapWindow;
}

class MapWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MapWindow(QWidget *parent = nullptr);
    ~MapWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MapWindow *ui;
};

#endif // MAP_WINDOW_H
