#include "trip_detail_page.h"
#include "ui_trip_detail_page.h"
#include "../cart_page/cart_page.h"
#include "../detail_window/detail_window.h"
#include "../App/application.h"

#include <QAbstractItemView>
#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPixmap>
#include <QResizeEvent>
#include <QStringList>
#include <QTableWidgetItem>

TripDetailPage::TripDetailPage(QWidget *parent)
    : QWidget(parent)
    , _ui(new Ui::TripDetailPage)
{
    _ui->setupUi(this);
    setFixedSize(1056, 686);
    _ui->lstTripStops->setStyleSheet(
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
    _ui->tblSouvenirs->setStyleSheet(
        "QTableWidget::item:hover {"
        "background: rgb(219, 234, 254);"
        "color: rgb(30, 41, 59);"
        "}"
        "QTableWidget::item:selected {"
        "background: rgb(191, 219, 254);"
        "color: rgb(30, 41, 59);"
        "}"
        );
    connect(_ui->btnViewCart, &QPushButton::clicked, this, &TripDetailPage::handleViewCartClick);
    connect(_ui->btnEndTrip, &QPushButton::clicked, this, &TripDetailPage::handleEndTripClick);
    connect(_ui->btnPreviousStop, &QPushButton::clicked, this, &TripDetailPage::handlePreviousStopClick);
    connect(_ui->btnNextStop, &QPushButton::clicked, this, &TripDetailPage::handleNextStopClick);
    connect(_ui->btnMoreInfo, &QPushButton::clicked, this, &TripDetailPage::handleMoreInfoClick);
    connect(_ui->btnAddToCart, &QPushButton::clicked, this, &TripDetailPage::handleAddToCartClick);
    connect(_ui->lstTripStops, &QListWidget::currentRowChanged,
            this, &TripDetailPage::handleTripStopsCurrentRowChange);

    _current_trip = APP->getTripPlanner()->getCurrentTrip();

    if (!_current_trip)
    {
        updateStadiumSummary();
        updateTripSummary();
        updateNavigationButtons();
        return;
    }

    _all_stadiums = &(_current_trip->getResult().stadiums);
    _transit_flags = &(_current_trip->getResult().transit_flags);
    loadTripStops();
    syncFromCurrentTrip();
}

TripDetailPage::~TripDetailPage()
{
    delete _ui;
}

void TripDetailPage::handleViewCartClick()
{
    CartPage* cart_page = new CartPage(_current_trip, this);
    cart_page->setAttribute(Qt::WA_DeleteOnClose);
    connect(cart_page, &CartPage::cartUpdated, this, [this]() {
        updateTripSummary();
    });
    cart_page->show();
}

void TripDetailPage::handleEndTripClick()
{
    if (_current_trip == nullptr)
    {
        close();
        return;
    }

    const QMessageBox::StandardButton response = QMessageBox::question(
        this,
        "End Trip",
        "End this trip and close the trip detail page?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (response != QMessageBox::Yes)
        return;

    showCartSummary("Trip Summary", true);
}

void TripDetailPage::handlePreviousStopClick()
{
    selectPreviousStop();
}

void TripDetailPage::handleNextStopClick()
{
    selectNextStop();
}

void TripDetailPage::handleMoreInfoClick()
{
    openMoreInfo();
}

void TripDetailPage::handleAddToCartClick()
{
    if (_current_trip == nullptr || !_has_current_stadium || APP == nullptr)
    {
        QMessageBox::warning(this, "Add to Cart", "No active trip stop is available.");
        return;
    }

    SouvenirRepository* souvenir_repo = APP->souvenirRepository();
    if (souvenir_repo == nullptr)
    {
        QMessageBox::warning(this, "Add to Cart", "Souvenir data is not available.");
        return;
    }

    const int current_row = _ui->tblSouvenirs->currentRow();
    if (current_row < 0)
    {
        QMessageBox::information(this, "Add to Cart", "Select a souvenir before adding it to the cart.");
        return;
    }

    const std::vector<Souvenir> souvenirs =
        souvenir_repo->getSouvenirsByStadiumID(_current_stadium->stadium_id);

    if (current_row >= static_cast<int>(souvenirs.size()))
    {
        QMessageBox::warning(this, "Add to Cart", "The selected souvenir could not be loaded.");
        return;
    }

    const Souvenir& selected_souvenir = souvenirs[current_row];
    const int quantity = _ui->spnQuantity->value();
    if (!_current_trip->addSouvenirToCart(selected_souvenir, quantity))
    {
        QMessageBox::warning(this, "Add to Cart", "Failed to add the selected souvenir to the cart.");
        return;
    }

    updateTripSummary();
    QMessageBox::information(
        this,
        "Added to Cart",
        QString("Added %1 x %2 to the cart.")
            .arg(quantity)
            .arg(selected_souvenir.name)
        );
}

void TripDetailPage::handleTripStopsCurrentRowChange(int currentRow)
{
    if (_is_syncing_selection)
        return;

    if (_current_trip == nullptr || _all_stadiums == nullptr)
    {
        return;
    }

    const int trip_row = static_cast<int>(_current_trip->currentStopIndex());
    if (currentRow != trip_row)
        syncFromCurrentTrip();
}

void TripDetailPage::loadTripStops()
{
    _ui->lstTripStops->clear();

    if (_all_stadiums == nullptr)
        return;

    for (int index = 0; index < static_cast<int>(_all_stadiums->size()); ++index)
    {
        const Stadium& stadium = (*_all_stadiums)[index];
        const bool is_transit =
            (_transit_flags != nullptr &&
             index < static_cast<int>(_transit_flags->size()) &&
             (*_transit_flags)[index]);
        const QString display_text = QString("%1. %2")
                                         .arg(index + 1)
                                         .arg(is_transit
                                                  ? stadium.stadium_name + " (Transit)"
                                                  : stadium.stadium_name);
        QListWidgetItem *item = new QListWidgetItem(display_text);
        if (is_transit)
        {
            QFont font = item->font();
            font.setItalic(true);
            item->setFont(font);
        }
        _ui->lstTripStops->addItem(item);
    }
}


void TripDetailPage::openMoreInfo()
{
    if (!_has_current_stadium)
    {
        QMessageBox::information(this, "No Stadium Selected",
                                 "Select a stadium before opening more information.");
        return;
    }

    DetailWindow *detail_window = new DetailWindow(*_current_stadium, this);
    detail_window->setAttribute(Qt::WA_DeleteOnClose);
    detail_window->show();
}

void TripDetailPage::syncFromCurrentTrip()
{
    if (_current_trip == nullptr || _all_stadiums == nullptr || !_current_trip->hasStops())
    {
        _current_stadium = nullptr;
        _has_current_stadium = false;
        _ui->lblTripProgress->setText(
            QString("Stop 0 of %1").arg(_all_stadiums == nullptr ? 0 : static_cast<int>(_all_stadiums->size()))
            );
        updateStadiumSummary();
        updateTripSummary();
        updateNavigationButtons();
        updateTripStopStyles();
        return;
    }

    const int current_row = static_cast<int>(_current_trip->currentStopIndex());

    _current_stadium = _current_trip->getCurrentStop();
    _has_current_stadium = (_current_stadium != nullptr);
    _ui->lblTripProgress->setText(
        QString("Stop %1 of %2").arg(current_row + 1).arg(static_cast<int>(_all_stadiums->size()))
        );

    _is_syncing_selection = true;
    _ui->lstTripStops->setCurrentRow(current_row);
    _is_syncing_selection = false;

    updateStadiumSummary();
    updateTripSummary();
    updateNavigationButtons();
    updateTripStopStyles();
}

void TripDetailPage::loadSouvenirs()
{
    _ui->tblSouvenirs->clearContents();
    _ui->tblSouvenirs->setRowCount(0);
    _ui->tblSouvenirs->setColumnCount(2);

    QStringList headers;
    headers << "Souvenir" << "Price";
    _ui->tblSouvenirs->setHorizontalHeaderLabels(headers);

    _ui->tblSouvenirs->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _ui->tblSouvenirs->setSelectionBehavior(QAbstractItemView::SelectRows);
    _ui->tblSouvenirs->setSelectionMode(QAbstractItemView::SingleSelection);
    _ui->tblSouvenirs->setAlternatingRowColors(true);
    _ui->tblSouvenirs->verticalHeader()->setVisible(false);
    _ui->tblSouvenirs->horizontalHeader()->setStretchLastSection(true);
    _ui->tblSouvenirs->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    if (!_has_current_stadium || APP == nullptr || APP->souvenirRepository() == nullptr)
    {
        _ui->btnAddToCart->setEnabled(false);
        return;
    }

    const std::vector<Souvenir> souvenirs =
        APP->souvenirRepository()->getSouvenirsByStadiumID(_current_stadium->stadium_id);

    _ui->tblSouvenirs->setRowCount(static_cast<int>(souvenirs.size()));

    for (int row = 0; row < static_cast<int>(souvenirs.size()); ++row)
    {
        const Souvenir& souvenir = souvenirs[row];
        _ui->tblSouvenirs->setItem(row, 0, new QTableWidgetItem(souvenir.name));
        _ui->tblSouvenirs->setItem(
            row, 1, new QTableWidgetItem("$" + QString::number(souvenir.price, 'f', 2))
            );
    }

    _ui->btnAddToCart->setEnabled(!souvenirs.empty());
    if (!souvenirs.empty())
        _ui->tblSouvenirs->setCurrentCell(0, 0);
}

void TripDetailPage::selectPreviousStop()
{
    if (_current_trip == nullptr)
        return;

    if (_current_trip->goBackLastStop())
        syncFromCurrentTrip();
}

void TripDetailPage::selectNextStop()
{
    if (_current_trip == nullptr)
        return;

    if (_current_trip->goNextStop())
        syncFromCurrentTrip();
}

void TripDetailPage::updateNavigationButtons()
{
    if (_current_trip == nullptr || !_current_trip->hasStops())
    {
        _ui->btnPreviousStop->setEnabled(false);
        _ui->btnNextStop->setEnabled(false);
        return;
    }

    _ui->btnPreviousStop->setEnabled(!_current_trip->isAtFirstStop());
    _ui->btnNextStop->setEnabled(!_current_trip->isAtLastStop());
}

void TripDetailPage::setElidedLabelText(QLabel *label, const QString& full_text)
{
    if (label == nullptr)
        return;

    const int available_width = qMax(0, label->width() - 4);
    const QString displayed_text = label->fontMetrics().elidedText(
        full_text,
        Qt::ElideRight,
        available_width
        );

    label->setText(displayed_text);
    label->setToolTip(full_text);
}

void TripDetailPage::updateDisplayedStadiumTexts()
{
    setElidedLabelText(_ui->lblSelectedStadium, _selected_stadium_text);
    setElidedLabelText(_ui->lblTeamNameValue, _team_name_text);
    setElidedLabelText(_ui->lblLeagueValue, _league_text);
    setElidedLabelText(_ui->lblLocationValue, _location_text);
}

void TripDetailPage::updateTripStopStyles()
{
    const int current_row = _ui->lstTripStops->currentRow();

    for (int index = 0; index < _ui->lstTripStops->count(); ++index)
    {
        QListWidgetItem *item = _ui->lstTripStops->item(index);
        if (item == nullptr)
            continue;

        if (current_row >= 0 && index < current_row)
        {
            item->setBackground(QColor(220, 252, 231));
            item->setForeground(QColor(22, 101, 52));
        }
        else if (index == current_row)
        {
            item->setBackground(QColor(191, 219, 254));
            item->setForeground(QColor(30, 41, 59));
        }
        else
        {
            item->setBackground(QColor(Qt::white));
            item->setForeground(QColor(Qt::black));
        }
    }
}

void TripDetailPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateDisplayedStadiumTexts();
}

void TripDetailPage::loadStadiumImage()
{
    if (!_has_current_stadium || APP == nullptr)
    {
        _ui->lblStadiumImage->setPixmap(QPixmap());
        _ui->lblStadiumImage->setText("No image available");
        _ui->lblStadiumImage->setAlignment(Qt::AlignCenter);
        return;
    }

    QString assets_path = APP->assetsDir() + "/images/stadiums/";
    QString image_path = assets_path + QString::number(_current_stadium->stadium_id) + ".jpg";

    QPixmap pixmap;

    if (QFile::exists(image_path) && pixmap.load(image_path))
    {
        _ui->lblStadiumImage->setPixmap(
            pixmap.scaled(_ui->lblStadiumImage->size(),
                          Qt::KeepAspectRatio,
                          Qt::SmoothTransformation)
            );
        _ui->lblStadiumImage->setAlignment(Qt::AlignCenter);
        _ui->lblStadiumImage->setText("");
    }
    else
    {
        _ui->lblStadiumImage->setPixmap(QPixmap());
        _ui->lblStadiumImage->setText("No image available");
        _ui->lblStadiumImage->setAlignment(Qt::AlignCenter);
    }
}

void TripDetailPage::updateStadiumSummary()
{
    if (!_has_current_stadium)
    {
        _selected_stadium_text = "No stadium selected";
        _team_name_text = "N/A";
        _league_text = "N/A";
        _location_text = "N/A";
        updateDisplayedStadiumTexts();
        _ui->btnMoreInfo->setEnabled(false);
        loadSouvenirs();
        loadStadiumImage();
        return;
    }

    _selected_stadium_text = _current_stadium->stadium_name;
    const size_t current_index = _current_trip == nullptr ? 0 : _current_trip->currentStopIndex();
    const bool is_transit =
        (_transit_flags != nullptr &&
         current_index < _transit_flags->size() &&
         (*_transit_flags)[current_index]);
    if (is_transit)
        _selected_stadium_text += " (Transit)";
    _team_name_text = _current_stadium->team_name;
    _league_text = _current_stadium->league;
    _location_text = _current_stadium->location;
    updateDisplayedStadiumTexts();
    _ui->btnMoreInfo->setEnabled(true);
    loadSouvenirs();
    loadStadiumImage();
}

void TripDetailPage::updateTripSummary()
{
    if (_current_trip == nullptr)
    {
        _ui->lblTripSummary->setText("Total Distance: 0.0 mi | Total Cost: $0.00");
        return;
    }

    const TripResult& result = _current_trip->getResult();
    _ui->lblTripSummary->setText(
        QString("Total Distance: %1 mi | Total Cost: $%2")
            .arg(QString::number(result.total_distance, 'f', 1))
            .arg(QString::number(_current_trip->totalCost(), 'f', 2))
        );
}

QString TripDetailPage::buildCartSummaryText(bool include_trip_totals) const
{
    if (_current_trip == nullptr || _current_trip->getShoppingCart() == nullptr)
        return "No active trip is available.";

    const ShoppingCart* cart = _current_trip->getShoppingCart();
    if (cart->empty())
    {
        QString empty_message = "Your shopping cart is empty.";
        if (include_trip_totals)
        {
            empty_message += QString("\n\nTrip Distance: %1 mi")
                                 .arg(QString::number(_current_trip->getResult().total_distance, 'f', 1));
        }
        return empty_message;
    }

    QStringList lines;
    std::vector<int> grouped_stadium_ids;

    for (const CartItem& cart_item : cart->items())
    {
        const int stadium_id = cart_item.item_souvenir.owner_stadium_id;
        bool already_grouped = false;

        for (int existing_id : grouped_stadium_ids)
        {
            if (existing_id == stadium_id)
            {
                already_grouped = true;
                break;
            }
        }

        if (!already_grouped)
            grouped_stadium_ids.push_back(stadium_id);
    }

    for (int stadium_id : grouped_stadium_ids)
    {
        lines << resolveStadiumName(stadium_id);

        for (const CartItem& cart_item : cart->items())
        {
            if (cart_item.item_souvenir.owner_stadium_id != stadium_id)
                continue;

            lines << QString("  %1 x %2 @ $%3")
                         .arg(cart_item.quantity)
                         .arg(cart_item.item_souvenir.name)
                         .arg(QString::number(cart_item.item_souvenir.price, 'f', 2));
        }

        lines << QString("  Stadium Total: $%1")
                     .arg(QString::number(cart->totalCostForStadium(stadium_id), 'f', 2));
        lines << "";
    }

    if (!lines.isEmpty() && lines.back().isEmpty())
        lines.removeLast();

    if (include_trip_totals)
    {
        lines << "";
        lines << QString("Trip Distance: %1 mi")
                     .arg(QString::number(_current_trip->getResult().total_distance, 'f', 1));
    }

    lines << QString("Items Purchased: %1").arg(cart->totalQuantity());
    lines << QString("Grand Total: $%1").arg(QString::number(cart->totalCost(), 'f', 2));

    return lines.join('\n');
}

QString TripDetailPage::resolveStadiumName(int stadium_id) const
{
    if (APP == nullptr || APP->stadiumRepository() == nullptr)
        return QString("Stadium #%1").arg(stadium_id);

    const std::optional<Stadium> stadium = APP->stadiumRepository()->getStadiumByID(stadium_id);
    if (!stadium.has_value())
        return QString("Stadium #%1").arg(stadium_id);

    return stadium->stadium_name;
}

void TripDetailPage::showCartSummary(const QString& title, bool close_after_showing)
{
    QMessageBox summary_box(this);
    summary_box.setWindowTitle(title);
    summary_box.setIcon(QMessageBox::Information);
    summary_box.setText(buildCartSummaryText(close_after_showing));
    summary_box.exec();

    if (close_after_showing)
        close();
}
