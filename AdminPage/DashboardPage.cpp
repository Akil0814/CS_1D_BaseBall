//
// Created by Erfan Tavassoli on 4/6/26.
//

#include "DashboardPage.h"

#include "stadium_adding/stadium_adding.h"
#include "ui_DashboardPage.h"

#include "App/application.h"
#include "App/utils/csv_utils.h"
#include "souvenir_adding/newsouvenirpopup.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHash>
#include <QHeaderView>
#include <QIntValidator>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMessageBox>
#include <QModelIndex>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSet>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStringList>

namespace
{
enum class CsvImportType
{
    Unknown,
    Stadium,
    Distance
};

constexpr int kPrimaryIdRole = Qt::UserRole + 1;
constexpr int kSecondaryIdRole = Qt::UserRole + 2;

CsvImportType detectCsvImportType(const QString& file_path)
{
    CsvUtils::Table csv;
    QString csv_error;
    if (!CsvUtils::readTable(file_path, csv, csv_error))
        return CsvImportType::Unknown;

    const QHash<QString, int> header_index = CsvUtils::buildHeaderIndex(csv.header);

    const bool looks_like_stadium_csv =
        CsvUtils::findHeaderIndex(header_index, { "team_name", "team" }) >= 0 &&
        CsvUtils::findHeaderIndex(header_index, { "stadium_name", "stadium" }) >= 0;

    const bool looks_like_distance_csv =
        CsvUtils::findHeaderIndex(header_index,
                                  { "originated_stadtium", "originated_stadium", "from_stadium", "from" }) >= 0 &&
        CsvUtils::findHeaderIndex(header_index,
                                  { "destination_stadtium", "destination_stadium", "to_stadium", "to" }) >= 0 &&
        CsvUtils::findHeaderIndex(header_index, { "distance", "mileage", "miles" }) >= 0;

    if (looks_like_stadium_csv)
        return CsvImportType::Stadium;

    if (looks_like_distance_csv)
        return CsvImportType::Distance;

    return CsvImportType::Unknown;
}

void appendImportMessage(QStringList& lines,
                         const QString& prefix,
                         const QString& file_name,
                         const QString& detail = QString())
{
    QString line = prefix + file_name;
    if (!detail.trimmed().isEmpty())
        line += " - " + detail.trimmed();

    lines.append(line);
}

std::optional<double> parseNonNegativeDouble(const QString& text)
{
    bool ok = false;
    const double value = text.trimmed().toDouble(&ok);
    if (!ok || value < 0.0)
        return std::nullopt;

    return value;
}
}

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DashboardPage)
{
    ui->setupUi(this);
    ui->detailsTabWidget->setCurrentIndex(0);

    setupComboBox();
    setupValidators();
    setupStadiumModel();
    setupStadiumNameField();
    setupSouvenirModel();
    setupDistanceModel();
    setupDetailsPanel();
    refreshConnections();

    connect(APP->databaseManager(), &DatabaseManager::databaseReset,
            this, &DashboardPage::refreshConnections);
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

void DashboardPage::setupStadiumModel()
{
    stadiumModel = new QStandardItemModel(this);
    stadiumModel->setColumnCount(1);
    ui->stadiumList->setModel(stadiumModel);

    connect(ui->stadiumList->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex& current, const QModelIndex&) {
                handleStadiumSelectionChanged(current);
            });
}

void DashboardPage::setupStadiumNameField()
{
    connect(ui->stadiumNameLineEdit, &QLineEdit::returnPressed, this, [this]() {
        updateField(2, ui->stadiumNameLineEdit->text());
    });
}

void DashboardPage::setupSouvenirModel()
{
    souvenirModel = new QStandardItemModel(this);
    souvenirModel->setColumnCount(2);
    souvenirModel->setHorizontalHeaderLabels({ "Souvenir Name", "Price ($)" });

    ui->souvenirTableView->setModel(souvenirModel);
    setupSouvenirTableFormatting();

    connect(souvenirModel, &QStandardItemModel::itemChanged, this, [this](QStandardItem* item) {
        if (_is_populating_models || item == nullptr)
            return;

        const int row = item->row();
        if (row < 0 || row >= static_cast<int>(_souvenirs.size()))
            return;

        Souvenir updated = _souvenirs[row];

        if (item->column() == 0)
        {
            const QString name = item->text().trimmed();
            if (name.isEmpty())
            {
                populateSouvenirModel(updated.owner_stadium_id);
                return;
            }

            updated.name = name;
        }
        else if (item->column() == 1)
        {
            const std::optional<double> price = parseNonNegativeDouble(item->text());
            if (!price.has_value())
            {
                populateSouvenirModel(updated.owner_stadium_id);
                return;
            }

            updated.price = *price;
        }
        else
            return;

        SouvenirRepository* repo = APP->souvenirRepository();
        if (repo == nullptr || !repo->updateSouvenir(updated))
        {
            QMessageBox::warning(this, "Update Souvenir", "Failed to update the selected souvenir.");
            populateSouvenirModel(updated.owner_stadium_id);
            return;
        }

        populateSouvenirModel(updated.owner_stadium_id);
    });
}

void DashboardPage::setupSouvenirTableFormatting()
{
    QHeaderView *header = ui->souvenirTableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
}

void DashboardPage::setupDistanceModel()
{
    distanceModel = new QStandardItemModel(this);
    distanceModel->setColumnCount(3);
    distanceModel->setHorizontalHeaderLabels({ "From Stadium", "To Stadium", "Distance (mi)" });

    ui->distancesTableView->setModel(distanceModel);
    setupDistanceTableFormatting();

    connect(distanceModel, &QStandardItemModel::itemChanged, this, [this](QStandardItem* item) {
        if (_is_populating_models || item == nullptr || item->column() != 2)
            return;

        const int row = item->row();
        if (row < 0 || row >= static_cast<int>(_distances.size()))
            return;

        const std::optional<double> updated_distance = parseNonNegativeDouble(item->text());
        if (!updated_distance.has_value())
        {
            populateDistanceModel(currentStadiumId().value_or(-1));
            return;
        }

        const DistanceEdge& edge = _distances[row];
        DistanceRepository* repo = APP->distanceRepository();
        if (repo == nullptr ||
            !repo->updateDistanceBetweenStadiums(edge.from_stadium_id, edge.to_stadium_id, *updated_distance))
        {
            QMessageBox::warning(this, "Update Distance", "Failed to update the selected distance.");
            populateDistanceModel(currentStadiumId().value_or(-1));
            return;
        }

        populateDistanceModel(currentStadiumId().value_or(-1));
    });
}

void DashboardPage::setupDistanceTableFormatting()
{
    QHeaderView *header = ui->distancesTableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
}

void DashboardPage::populateStadiumModel(int selected_stadium_id)
{
    stadiumModel->removeRows(0, stadiumModel->rowCount());
    _stadiums.clear();

    StadiumRepository* repo = APP->stadiumRepository();
    if (repo == nullptr)
    {
        populateDetailsPanel(nullptr);
        populateSouvenirModel(-1);
        populateDistanceModel(-1);
        return;
    }

    _stadiums = repo->getAllStadiums(StadiumRepository::StadiumSortBy::TeamName);

    for (const Stadium& stadium : _stadiums)
    {
        auto *item = new QStandardItem(stadiumDisplayText(stadium));
        item->setEditable(false);
        item->setData(stadium.stadium_id, kPrimaryIdRole);
        stadiumModel->appendRow(item);
    }

    if (_stadiums.empty())
    {
        populateDetailsPanel(nullptr);
        populateSouvenirModel(-1);
        populateDistanceModel(-1);
        return;
    }

    int target_row = 0;
    for (int row = 0; row < stadiumModel->rowCount(); ++row)
    {
        if (stadiumModel->data(stadiumModel->index(row, 0), kPrimaryIdRole).toInt() == selected_stadium_id)
        {
            target_row = row;
            break;
        }
    }

    ui->stadiumList->setCurrentIndex(stadiumModel->index(target_row, 0));
}

void DashboardPage::populateSouvenirModel(int stadium_id)
{
    _is_populating_models = true;
    souvenirModel->removeRows(0, souvenirModel->rowCount());
    _souvenirs.clear();

    SouvenirRepository* repo = APP->souvenirRepository();
    if (repo != nullptr && stadium_id > 0)
        _souvenirs = repo->getSouvenirsByStadiumID(stadium_id);

    for (const Souvenir& souvenir : _souvenirs)
    {
        auto *name_item = new QStandardItem(souvenir.name);
        auto *price_item = new QStandardItem(QString::number(souvenir.price, 'f', 2));

        name_item->setData(souvenir.souvenir_id, kPrimaryIdRole);
        price_item->setData(souvenir.souvenir_id, kPrimaryIdRole);

        QList<QStandardItem*> row_items;
        row_items << name_item << price_item;
        souvenirModel->appendRow(row_items);
    }

    _is_populating_models = false;
}

void DashboardPage::populateDistanceModel(int stadium_id)
{
    _is_populating_models = true;
    distanceModel->removeRows(0, distanceModel->rowCount());
    _distances.clear();

    DistanceRepository* repo = APP->distanceRepository();
    if (repo != nullptr && stadium_id > 0)
    {
        const std::optional<DistanceNode> node = repo->getDistanceNodeOfStadium(stadium_id);
        if (node.has_value())
            _distances = node->edges;
    }

    const QString from_name = stadiumNameById(stadium_id);

    for (const DistanceEdge& edge : _distances)
    {
        auto *from_item = new QStandardItem(from_name);
        auto *to_item = new QStandardItem(stadiumNameById(edge.to_stadium_id));
        auto *distance_item = new QStandardItem(QString::number(edge.distance, 'f', 1));

        from_item->setEditable(false);
        to_item->setEditable(false);

        from_item->setData(edge.from_stadium_id, kPrimaryIdRole);
        from_item->setData(edge.to_stadium_id, kSecondaryIdRole);
        to_item->setData(edge.from_stadium_id, kPrimaryIdRole);
        to_item->setData(edge.to_stadium_id, kSecondaryIdRole);
        distance_item->setData(edge.from_stadium_id, kPrimaryIdRole);
        distance_item->setData(edge.to_stadium_id, kSecondaryIdRole);

        QList<QStandardItem*> row_items;
        row_items << from_item << to_item << distance_item;
        distanceModel->appendRow(row_items);
    }

    _is_populating_models = false;
}

void DashboardPage::populateDetailsPanel(const Stadium* stadium)
{
    _is_loading_details = true;

    const QSignalBlocker block_stadium_name(ui->stadiumNameLineEdit);
    const QSignalBlocker block_team_name(ui->teamNameLineEdit);
    const QSignalBlocker block_location(ui->locationLineEdit);
    const QSignalBlocker block_league(ui->leagueComboBox);
    const QSignalBlocker block_typology(ui->typologyComboBox);
    const QSignalBlocker block_surface(ui->surfaceComboBox);
    const QSignalBlocker block_roof(ui->roofComboBox);
    const QSignalBlocker block_capacity(ui->seatingCapacitySpinBox);
    const QSignalBlocker block_date_opened(ui->dateOpenedSpinBox);
    const QSignalBlocker block_center_field(ui->centerFieldSpinBox);

    if (stadium == nullptr)
    {
        ui->stadiumNameLineEdit->clear();
        ui->teamNameLineEdit->clear();
        ui->locationLineEdit->clear();
        ui->leagueComboBox->setCurrentIndex(0);
        ui->typologyComboBox->setCurrentIndex(0);
        ui->surfaceComboBox->setCurrentIndex(0);
        ui->roofComboBox->setCurrentIndex(0);
        ui->seatingCapacitySpinBox->setValue(0);
        ui->dateOpenedSpinBox->setValue(ui->dateOpenedSpinBox->minimum());
        ui->centerFieldSpinBox->setValue(0);
        _is_loading_details = false;
        return;
    }

    ui->stadiumNameLineEdit->setText(stadium->stadium_name);
    ui->teamNameLineEdit->setText(stadium->team_name);
    ui->locationLineEdit->setText(stadium->location);
    ui->leagueComboBox->setCurrentText(stadium->league);
    ui->typologyComboBox->setCurrentText(stadium->ballpark_typology);
    ui->surfaceComboBox->setCurrentText(stadium->playing_surface);
    ui->roofComboBox->setCurrentText(stadium->roof_type);
    ui->seatingCapacitySpinBox->setValue(stadium->seating_capacity);
    ui->dateOpenedSpinBox->setValue(stadium->date_opened);
    ui->centerFieldSpinBox->setValue(stadium->distance_to_center_field_ft);

    _is_loading_details = false;
}

void DashboardPage::handleStadiumSelectionChanged(const QModelIndex& current_index)
{
    if (!current_index.isValid())
    {
        populateDetailsPanel(nullptr);
        populateSouvenirModel(-1);
        populateDistanceModel(-1);
        return;
    }

    const int stadium_id = current_index.data(kPrimaryIdRole).toInt();
    const std::optional<Stadium> selected_stadium = currentStadium();

    populateDetailsPanel(selected_stadium ? &selected_stadium.value() : nullptr);
    populateSouvenirModel(stadium_id);
    populateDistanceModel(stadium_id);
}

std::optional<int> DashboardPage::currentStadiumId() const
{
    const QModelIndex current_index = ui->stadiumList->currentIndex();
    if (!current_index.isValid())
        return std::nullopt;

    const int stadium_id = current_index.data(kPrimaryIdRole).toInt();
    if (stadium_id <= 0)
        return std::nullopt;

    return stadium_id;
}

std::optional<Stadium> DashboardPage::currentStadium() const
{
    const std::optional<int> stadium_id = currentStadiumId();
    if (!stadium_id.has_value())
        return std::nullopt;

    for (const Stadium& stadium : _stadiums)
    {
        if (stadium.stadium_id == *stadium_id)
            return stadium;
    }

    return std::nullopt;
}

QString DashboardPage::stadiumDisplayText(const Stadium& stadium) const
{
    return QString("%1 - %2").arg(stadium.team_name, stadium.stadium_name);
}

QString DashboardPage::stadiumNameById(int stadium_id) const
{
    for (const Stadium& stadium : _stadiums)
    {
        if (stadium.stadium_id == stadium_id)
            return stadium.stadium_name;
    }

    StadiumRepository* repo = APP->stadiumRepository();
    if (repo == nullptr)
        return QString("Stadium #%1").arg(stadium_id);

    const std::optional<Stadium> stadium = repo->getStadiumByID(stadium_id);
    if (!stadium.has_value())
        return QString("Stadium #%1").arg(stadium_id);

    return stadium->stadium_name;
}

void DashboardPage::on_addSouvenirButton_clicked()
{
    const std::optional<int> stadium_id = currentStadiumId();
    if (!stadium_id.has_value())
        return;

    newSouvenirPopup dialog(this);

    if (dialog.exec() != QDialog::Accepted)
        return;

    Souvenir item;
    item.owner_stadium_id = *stadium_id;
    item.name = dialog.getName();
    item.price = dialog.getPrice();

    SouvenirRepository* repo = APP->souvenirRepository();
    if (repo == nullptr || !repo->addSouvenir(*stadium_id, item))
    {
        QMessageBox::warning(this, "Add Souvenir", "Failed to add the new souvenir.");
        return;
    }

    populateSouvenirModel(*stadium_id);
}

void DashboardPage::on_removeSouvenirButton_clicked()
{
    const QModelIndex current_index = ui->souvenirTableView->currentIndex();
    if (!current_index.isValid())
        return;

    const int row = current_index.row();
    if (row < 0 || row >= static_cast<int>(_souvenirs.size()))
        return;

    SouvenirRepository* repo = APP->souvenirRepository();
    if (repo == nullptr || !repo->deleteSouvenir(_souvenirs[row].souvenir_id))
    {
        QMessageBox::warning(this, "Remove Souvenir", "Failed to remove the selected souvenir.");
        return;
    }

    populateSouvenirModel(currentStadiumId().value_or(-1));
}

void DashboardPage::on_uploadFile_clicked()
{
    if (APP == nullptr || APP->databaseManager() == nullptr)
    {
        QMessageBox::warning(this, "Import Files", "Database manager is not available.");
        return;
    }

    const QStringList file_paths = QFileDialog::getOpenFileNames(
        this,
        "Select CSV Files to Import",
        APP->assetsDir().trimmed().isEmpty() ? QString() : APP->assetsDir(),
        "CSV Files (*.csv);;All Files (*.*)"
        );

    if (file_paths.isEmpty())
        return;

    QStringList stadium_files;
    QStringList distance_files;
    QStringList result_lines;

    for (const QString& file_path : file_paths)
    {
        const CsvImportType import_type = detectCsvImportType(file_path);
        const QString file_name = QFileInfo(file_path).fileName();

        switch (import_type)
        {
        case CsvImportType::Stadium:
            stadium_files.append(file_path);
            break;
        case CsvImportType::Distance:
            distance_files.append(file_path);
            break;
        case CsvImportType::Unknown:
        default:
            appendImportMessage(
                result_lines,
                "Skipped unsupported file: ",
                file_name,
                "Header did not match stadium or distance import format."
                );
            break;
        }
    }

    bool imported_anything = false;

    auto import_files = [&](const QStringList& paths, bool is_stadium_import)
    {
        for (const QString& path : paths)
        {
            const QString file_name = QFileInfo(path).fileName();
            const bool ok = is_stadium_import
                ? APP->databaseManager()->importStadiumsFromFile(path)
                : APP->databaseManager()->importDistancesFromFile(path);

            const QString detail = ok
                ? APP->databaseManager()->lastWarning()
                : APP->databaseManager()->lastError();

            appendImportMessage(
                result_lines,
                ok ? "Imported: " : "Failed: ",
                file_name,
                detail
                );

            if (ok)
                imported_anything = true;
        }
    };

    import_files(stadium_files, true);
    import_files(distance_files, false);

    if (imported_anything)
        refreshConnections();

    QMessageBox message_box(this);
    message_box.setWindowTitle("Import Files");
    message_box.setIcon(imported_anything ? QMessageBox::Information : QMessageBox::Warning);
    message_box.setText(result_lines.isEmpty()
                            ? "No importable files were selected."
                            : result_lines.join('\n'));
    message_box.exec();
}

void DashboardPage::refreshConnections()
{
    populateStadiumModel(currentStadiumId().value_or(-1));
}

void DashboardPage::on_removeStadiumButton_clicked()
{
    const std::optional<int> stadium_id = currentStadiumId();
    if (!stadium_id.has_value())
        return;

    StadiumRepository* repo = APP->stadiumRepository();
    if (repo == nullptr || !repo->deleteStadium(*stadium_id))
    {
        QMessageBox::warning(this, "Remove Stadium", "Failed to remove the selected stadium.");
        return;
    }

    refreshConnections();
}

void DashboardPage::on_addStadiumButton_clicked()
{
    StadiumAdding dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    StadiumRepository* repo = APP->stadiumRepository();
    if (repo == nullptr || !repo->addStadium(dialog.getFormData()))
    {
        QMessageBox::warning(this, "Add Stadium", "Failed to add the new stadium.");
        return;
    }

    refreshConnections();
}

void DashboardPage::setupComboBox()
{
    ui->leagueComboBox->addItem("National");
    ui->leagueComboBox->addItem("American");

    ui->typologyComboBox->addItem("Retro Modern");
    ui->typologyComboBox->addItem("Retro Classic");
    ui->typologyComboBox->addItem("Jewel Box");
    ui->typologyComboBox->addItem("Modern");
    ui->typologyComboBox->addItem("Contemporary");
    ui->typologyComboBox->addItem("Multipurpose");

    ui->surfaceComboBox->addItem("Grass");
    ui->surfaceComboBox->addItem("AstroTurf GameDay Grass");
    ui->surfaceComboBox->addItem("AstroTurf GameDay Grass 3D");

    ui->roofComboBox->addItem("Open");
    ui->roofComboBox->addItem("Retractable");
}

void DashboardPage::setupValidators()
{
    QRegularExpression nameRegex("^[a-zA-Z\\s\\-.]{1,100}$");
    QRegularExpression locRegex("^[a-zA-Z\\s,\\.\\-]{1,1000}$");

    auto *nameValidator = new QRegularExpressionValidator(nameRegex, this);
    auto *locValidator = new QRegularExpressionValidator(locRegex, this);

    ui->stadiumNameLineEdit->setValidator(nameValidator);
    ui->teamNameLineEdit->setValidator(nameValidator);
    ui->locationLineEdit->setValidator(locValidator);

    ui->seatingCapacitySpinBox->setRange(0, 200000);
    ui->dateOpenedSpinBox->setRange(1800, 2100);

    const QList<QLineEdit*> line_edits = { ui->stadiumNameLineEdit, ui->teamNameLineEdit, ui->locationLineEdit };
    for (QLineEdit* edit : line_edits)
    {
        connect(edit, &QLineEdit::textChanged, [edit]() {
            if (edit->hasAcceptableInput() || edit->text().isEmpty())
                edit->setStyleSheet("");
            else
                edit->setStyleSheet("border: 1px solid red; background-color: #FFF0F0;");
        });
    }
}

void DashboardPage::setupDetailsPanel()
{
    ui->centerFieldSpinBox->setMaximum(1000000);

    connect(ui->teamNameLineEdit, &QLineEdit::returnPressed, this, [this]() {
        updateField(1, ui->teamNameLineEdit->text());
    });

    connect(ui->locationLineEdit, &QLineEdit::returnPressed, this, [this]() {
        updateField(4, ui->locationLineEdit->text());
    });

    connect(ui->leagueComboBox, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        updateField(6, text);
    });

    connect(ui->surfaceComboBox, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        updateField(5, text);
    });

    connect(ui->typologyComboBox, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        updateField(10, text);
    });

    connect(ui->roofComboBox, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        updateField(11, text);
    });

    void (QSpinBox::*valueChanged)(int) = &QSpinBox::valueChanged;

    connect(ui->seatingCapacitySpinBox, valueChanged, this, [this](int value) {
        updateField(3, value);
    });

    connect(ui->dateOpenedSpinBox, valueChanged, this, [this](int value) {
        updateField(7, value);
    });

    connect(ui->centerFieldSpinBox, valueChanged, this, [this](int value) {
        updateField(8, value);
    });
}

void DashboardPage::updateField(int columnIdx, const QVariant &value)
{
    if (_is_loading_details)
        return;

    const std::optional<Stadium> selected_stadium = currentStadium();
    if (!selected_stadium.has_value())
        return;

    Stadium updated = *selected_stadium;

    switch (columnIdx)
    {
    case 1:
        updated.team_name = value.toString().trimmed();
        break;
    case 2:
        updated.stadium_name = value.toString().trimmed();
        break;
    case 3:
        updated.seating_capacity = value.toInt();
        break;
    case 4:
        updated.location = value.toString().trimmed();
        break;
    case 5:
        updated.playing_surface = value.toString().trimmed();
        break;
    case 6:
        updated.league = value.toString().trimmed();
        break;
    case 7:
        updated.date_opened = value.toInt();
        break;
    case 8:
        updated.distance_to_center_field_ft = value.toInt();
        updated.distance_to_center_field_raw = QString::number(updated.distance_to_center_field_ft);
        break;
    case 10:
        updated.ballpark_typology = value.toString().trimmed();
        break;
    case 11:
        updated.roof_type = value.toString().trimmed();
        break;
    default:
        return;
    }

    if (updated.team_name.isEmpty() || updated.stadium_name.isEmpty())
    {
        QMessageBox::warning(this, "Update Stadium", "Team name and stadium name cannot be empty.");
        populateDetailsPanel(&selected_stadium.value());
        return;
    }

    StadiumRepository* repo = APP->stadiumRepository();
    if (repo == nullptr || !repo->upDateStadiumInform(updated.stadium_id, updated))
    {
        QMessageBox::warning(this, "Update Stadium", "Failed to update the selected stadium.");
        refreshConnections();
        return;
    }

    populateStadiumModel(updated.stadium_id);
}

void DashboardPage::on_removeDistanceButton_clicked()
{
    const QModelIndex current_index = ui->distancesTableView->currentIndex();
    if (!current_index.isValid())
        return;

    const int row = current_index.row();
    if (row < 0 || row >= static_cast<int>(_distances.size()))
        return;

    const DistanceEdge& edge = _distances[row];
    DistanceRepository* repo = APP->distanceRepository();
    if (repo == nullptr || !repo->removeDistanceBetweenStadiums(edge.from_stadium_id, edge.to_stadium_id))
    {
        QMessageBox::warning(this, "Remove Distance", "Failed to remove the selected distance.");
        return;
    }

    populateDistanceModel(currentStadiumId().value_or(-1));
}

void DashboardPage::on_addDistanceButton_clicked()
{
    const std::optional<Stadium> origin = currentStadium();
    if (!origin.has_value())
    {
        QMessageBox::warning(this, "Selection Required", "Please select an originating stadium from the list first.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Add New Distance from " + origin->stadium_name);

    QFormLayout form(&dialog);
    auto *stadium_combo = new QComboBox(&dialog);
    auto *distance_edit = new QLineEdit(&dialog);
    distance_edit->setValidator(new QIntValidator(1, 9999, &dialog));
    distance_edit->setPlaceholderText("Miles");

    QSet<int> existing_dest_ids;
    for (const DistanceEdge& edge : _distances)
        existing_dest_ids.insert(edge.to_stadium_id);

    for (const Stadium& stadium : _stadiums)
    {
        if (stadium.stadium_id != origin->stadium_id && !existing_dest_ids.contains(stadium.stadium_id))
            stadium_combo->addItem(stadium.stadium_name, stadium.stadium_id);
    }

    if (stadium_combo->count() == 0)
    {
        QMessageBox::information(this, "No Options", "All possible stadium connections already exist.");
        return;
    }

    form.addRow("To Stadium:", stadium_combo);
    form.addRow("Distance (mi):", distance_edit);

    QDialogButtonBox button_box(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&button_box);
    connect(&button_box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&button_box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    const int destination_id = stadium_combo->currentData().toInt();
    const int distance = distance_edit->text().toInt();

    if (distance <= 0)
    {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid distance.");
        return;
    }

    DistanceRepository* repo = APP->distanceRepository();
    if (repo == nullptr || !repo->addDistanceBetweenStadiums(origin->stadium_id, destination_id, distance))
    {
        QMessageBox::critical(this, "Database Error", "Failed to insert distance into the database.");
        return;
    }

    populateDistanceModel(origin->stadium_id);
}
