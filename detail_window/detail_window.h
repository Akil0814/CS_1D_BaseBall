#ifndef DETAIL_WINDOW_H
#define DETAIL_WINDOW_H

#include <QMainWindow>
<<<<<<< Updated upstream
=======
#include <vector>
>>>>>>> Stashed changes
#include "../data_types.h"

namespace Ui {
class detail_window;
}

class DetailWindow : public QMainWindow
{
    Q_OBJECT

public:
<<<<<<< Updated upstream
    explicit DetailWindow(QWidget *parent = nullptr);
=======
>>>>>>> Stashed changes
    explicit DetailWindow(const Stadium& stadium, QWidget *parent = nullptr);
    ~DetailWindow();

    void setStadium(const Stadium& stadium);

private:
    void loadStadiumDetails();
    void loadSouvenirs();

private:
    Ui::detail_window *ui;
    Stadium currentStadium;
};

#endif // DETAIL_WINDOW_H
