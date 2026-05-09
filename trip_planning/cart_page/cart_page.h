#ifndef CART_PAGE_H
#define CART_PAGE_H

#include "../../App/model/trip.h"

#include <QMainWindow>

class QLabel;
class QListWidget;
class QPushButton;

class CartPage : public QMainWindow
{
    Q_OBJECT

public:
    explicit CartPage(Trip* trip, QWidget *parent = nullptr);
    ~CartPage() override = default;

signals:
    void cartUpdated();

private slots:
    void handleSelectionChanged();
    void handleRemoveSelected();

private:
    void loadCartItems();
    void updateSummary();
    QString buildItemText(const CartItem& cart_item) const;
    QString resolveStadiumName(int stadium_id) const;

private:
    Trip* _trip = nullptr;
    QListWidget* _cart_list = nullptr;
    QLabel* _summary_label = nullptr;
    QPushButton* _remove_button = nullptr;
    QPushButton* _close_button = nullptr;
};

#endif // CART_PAGE_H
