#include "end_trip_summary.h"

#include <QHeaderView>
#include <QStringListModel>
#include <QStandardItemModel>
#include "application.h"
#include "ui_end_trip_summary.h"

end_trip_summary::end_trip_summary(QWidget *parent)
    : QWidget(parent), ui(new Ui::end_trip_summary) {
    ui->setupUi(this);

    setupStops();
    setupPurchases();
}

end_trip_summary::~end_trip_summary() { delete ui; }


void end_trip_summary::setupStops() {
    const auto tripResults = _current_trip->getResult();

    // Convert string to qstring
    QStringList stopsList;
    stopsModel = new QStringListModel(this);
    for (const auto& item : tripResults.stadiums) {
        stopsList.append(item.stadium_name + " / " + item.team_name);
    }

    stopsModel->setStringList(stopsList);

    ui->stopsListView->setModel(stopsModel);
    ui->totalDistValue->setText("Total Distance: " + QString::number(tripResults.total_distance) + " miles");
    ui->stopCountValue->setText("Number of Stops: " + QString::number(tripResults.stadiums.size()));
}

void end_trip_summary::setupPurchases() {
    // 1. Initialize the model with column headers
    cartModel = new QStandardItemModel(this);
    cartModel->setHorizontalHeaderLabels({"Stadium", "Souvenir", "Qty", "Price", "Total"});

    // 2. Get your shopping cart items (Assuming you have a list of CartItem)
    const auto& items = _current_trip->getShoppingCart()->items();

    for (const auto&[item_souvenir, quantity] : items) {
        const double line_total = item_souvenir.price * quantity;

        // Create individual items for each column
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(resolveStadiumName(item_souvenir.owner_stadium_id));
        rowItems << new QStandardItem(item_souvenir.name);
        rowItems << new QStandardItem(QString::number(quantity));
        rowItems << new QStandardItem("$" + QString::number(item_souvenir.price, 'f', 2));
        rowItems << new QStandardItem("$" + QString::number(line_total, 'f', 2));

        // Append the whole row to the model
        cartModel->appendRow(rowItems);
    }

    // 3. Set the model to your view (Use QTreeView or QTableView)
    ui->purchasesTableView->setModel(cartModel);

    // Optional: Make it look nice
    // 1. First, set the default behavior for all columns to Stretch (for Stadium and Souvenir)
    ui->purchasesTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 2. Override the specific indices for Qty, Price, and Total to fit the text content
    ui->purchasesTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Qty
    ui->purchasesTableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Price
    ui->purchasesTableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents); // Total

    // replace the placeholder text
    ui->totalCostValue->setText("Total Cost: $" + QString::number(_current_trip->totalCost()));
    ui->numItemsValue->setText("Number of Souvenirs: " + QString::number(_current_trip->getShoppingCart()->totalQuantity()));
}

QString end_trip_summary::resolveStadiumName(int stadium_id) const
{
    if (APP == nullptr || APP->stadiumRepository() == nullptr)
        return QString("Stadium #%1").arg(stadium_id);

    const std::optional<Stadium> stadium = APP->stadiumRepository()->getStadiumByID(stadium_id);
    if (!stadium.has_value())
        return QString("Stadium #%1").arg(stadium_id);

    return stadium->stadium_name;
}
