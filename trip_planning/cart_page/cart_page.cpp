#include "cart_page.h"

#include "../../App/application.h"

#include <QAbstractItemView>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include <optional>

CartPage::CartPage(Trip* trip, QWidget *parent)
    : QMainWindow(parent)
    , _trip(trip)
{
    setWindowTitle("Shopping Cart");
    resize(720, 520);

    QWidget* central_widget = new QWidget(this);
    QVBoxLayout* root_layout = new QVBoxLayout(central_widget);
    root_layout->setContentsMargins(16, 16, 16, 16);
    root_layout->setSpacing(12);

    QLabel* title_label = new QLabel("Shopping Cart", central_widget);
    QFont title_font = title_label->font();
    title_font.setPointSize(18);
    title_font.setBold(true);
    title_label->setFont(title_font);
    root_layout->addWidget(title_label);

    _cart_list = new QListWidget(central_widget);
    _cart_list->setSelectionMode(QAbstractItemView::SingleSelection);
    _cart_list->setAlternatingRowColors(true);
    _cart_list->setStyleSheet(
        "QListWidget::item {"
        "padding: 6px 8px;"
        "border-radius: 6px;"
        "}"
        "QListWidget::item:hover {"
        "background: rgb(219, 234, 254);"
        "color: rgb(30, 41, 59);"
        "}"
        "QListWidget::item:selected {"
        "background: rgb(191, 219, 254);"
        "color: rgb(30, 41, 59);"
        "}"
        );
    root_layout->addWidget(_cart_list, 1);

    _summary_label = new QLabel(central_widget);
    _summary_label->setWordWrap(true);
    root_layout->addWidget(_summary_label);

    QHBoxLayout* action_layout = new QHBoxLayout();
    action_layout->addStretch();

    _remove_button = new QPushButton("Remove Selected Item", central_widget);
    _close_button = new QPushButton("Close", central_widget);
    action_layout->addWidget(_remove_button);
    action_layout->addWidget(_close_button);
    root_layout->addLayout(action_layout);

    setCentralWidget(central_widget);

    connect(_cart_list, &QListWidget::currentRowChanged, this, &CartPage::handleSelectionChanged);
    connect(_remove_button, &QPushButton::clicked, this, &CartPage::handleRemoveSelected);
    connect(_close_button, &QPushButton::clicked, this, &CartPage::close);

    loadCartItems();
}

void CartPage::handleSelectionChanged()
{
    _remove_button->setEnabled(_cart_list != nullptr && _cart_list->currentRow() >= 0);
}

void CartPage::handleRemoveSelected()
{
    if (_trip == nullptr || _cart_list == nullptr)
        return;

    const int current_row = _cart_list->currentRow();
    if (current_row < 0)
        return;

    const ShoppingCart* cart = _trip->getShoppingCart();
    if (cart == nullptr || current_row >= static_cast<int>(cart->items().size()))
        return;

    const CartItem selected_item = cart->items()[current_row];
    const QMessageBox::StandardButton response = QMessageBox::question(
        this,
        "Remove Item",
        QString("Remove %1 from the shopping cart?").arg(selected_item.item_souvenir.name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (response != QMessageBox::Yes)
        return;

    if (!_trip->removeCartItemAt(static_cast<size_t>(current_row)))
    {
        QMessageBox::warning(this, "Remove Item", "Failed to remove the selected item.");
        return;
    }

    loadCartItems();
    emit cartUpdated();
}

void CartPage::loadCartItems()
{
    _cart_list->clear();

    if (_trip == nullptr || _trip->getShoppingCart() == nullptr)
    {
        _summary_label->setText("No active trip is available.");
        _remove_button->setEnabled(false);
        return;
    }

    const ShoppingCart* cart = _trip->getShoppingCart();
    for (const CartItem& cart_item : cart->items())
        _cart_list->addItem(buildItemText(cart_item));

    if (_cart_list->count() > 0)
        _cart_list->setCurrentRow(0);

    updateSummary();
    handleSelectionChanged();
}

void CartPage::updateSummary()
{
    if (_trip == nullptr || _trip->getShoppingCart() == nullptr)
    {
        _summary_label->setText("No active trip is available.");
        return;
    }

    const ShoppingCart* cart = _trip->getShoppingCart();
    _summary_label->setText(
        QString("Items: %1 | Total Quantity: %2 | Total Cost: $%3")
            .arg(cart->items().size())
            .arg(cart->totalQuantity())
            .arg(QString::number(cart->totalCost(), 'f', 2))
        );
}

QString CartPage::buildItemText(const CartItem& cart_item) const
{
    const double line_total = cart_item.item_souvenir.price * cart_item.quantity;
    return QString("%1 | %2 | Qty: %3 | $%4 each | Line Total: $%5")
        .arg(resolveStadiumName(cart_item.item_souvenir.owner_stadium_id))
        .arg(cart_item.item_souvenir.name)
        .arg(cart_item.quantity)
        .arg(QString::number(cart_item.item_souvenir.price, 'f', 2))
        .arg(QString::number(line_total, 'f', 2));
}

QString CartPage::resolveStadiumName(int stadium_id) const
{
    if (APP == nullptr || APP->stadiumRepository() == nullptr)
        return QString("Stadium #%1").arg(stadium_id);

    const std::optional<Stadium> stadium = APP->stadiumRepository()->getStadiumByID(stadium_id);
    if (!stadium.has_value())
        return QString("Stadium #%1").arg(stadium_id);

    return stadium->stadium_name;
}
