 //
// Created by Erfan Tavassoli on 4/6/26.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DashboardPage.h" resolved

#include "DashboardPage.h"
#include <QDialogButtonBox>
#include <QIntValidator>
#include <QSet>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHash>
#include <QSqlError>
#include <QSqlRecord>
#include <QStringList>
#include "ui_DashboardPage.h"
#include "App/application.h"
#include "App/utils/csv_utils.h"
#include <QSqlTableModel>
#include <QMessageBox>
#include <souvenir_adding/newsouvenirpopup.h>
#include <QStyledItemDelegate>

class StadiumNameDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QString displayText(const QVariant &value, const QLocale &locale) const override {
        int id = value.toInt();
        auto stadium = APP->stadiumRepository()->getStadiumByID(id);
        return stadium ? stadium->stadium_name : "Unknown Stadium";
    }
};
namespace
{
enum class CsvImportType
{
    Unknown,
    Stadium,
    Distance
};

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
}
#include "stadium_adding/stadium_adding.h"

DashboardPage::DashboardPage(QWidget *parent) :
QWidget(parent), ui(new Ui::DashboardPage) {
    ui->setupUi(this);

    const QSqlDatabase db = APP->databaseManager()->getDatabaseObj();

    ui->detailsTabWidget->setCurrentIndex(0);

    setupComboBox();
    setupValidators();
    setupDetailsPanel();


    refreshConnections();

    connect(APP->databaseManager(), &DatabaseManager::databaseReset,
        this, &DashboardPage::refreshConnections);
}

DashboardPage::~DashboardPage() {
    delete ui;
}

void DashboardPage::linkStadiumDB(const QSqlDatabase &db) {
    setupStadiumModel(db);

    ui->stadiumList->setModel(stadiumModel);
    ui->stadiumList->setModelColumn(2);

    setupStadiumNameField();
}
void DashboardPage::setupStadiumModel(const QSqlDatabase &db) {
    // create model
    stadiumModel = new QSqlTableModel(this, db);
    stadiumModel->setTable("stadiums");

    if (!stadiumModel->select()) {
        qDebug() << "Model Error:" << stadiumModel->lastError().text();
    }

    if (!APP->databaseManager()->isStadiumModuleAvailable()) {
        qDebug() << "Data is not loaded yet!";
    }

    stadiumModel->setEditStrategy(QSqlTableModel::OnFieldChange);
}
void DashboardPage::setupStadiumNameField() {
    // sync the lineedit when clicking on an index
    connect(ui->stadiumList, &QListView::clicked, this, [this](const QModelIndex &index) {
        // Get the full record for the row you just clicked
        QSqlRecord record = stadiumModel->record(index.row());

        // Update the LineEdit with the name
        QString name = record.value("stadium_name").toString(); // Use your actual column name
        ui->stadiumNameLineEdit->setText(name);
    });

    // Handle live-editing the name back to the DB
    connect(ui->stadiumNameLineEdit, &QLineEdit::textEdited, this, [this](const QString &newText) {
        QModelIndex currentIndex = ui->stadiumList->currentIndex();
        if (currentIndex.isValid()) {
            // This updates the model, which updates the DB because of OnFieldChange
            stadiumModel->setData(stadiumModel->index(currentIndex.row(), 2), newText);
        }
    });

    // When the user presses Enter in the LineEdit
    connect(ui->stadiumNameLineEdit, &QLineEdit::returnPressed, this, [this]() {
        QModelIndex currentIndex = ui->stadiumList->currentIndex();
        if (currentIndex.isValid()) {
            // Create an index specifically for Column 1 (campusName)
            QModelIndex nameIndex = stadiumModel->index(currentIndex.row(), 1);

            QString newName = ui->stadiumNameLineEdit->text();

            // Update the name, not the ID!
            if (stadiumModel->setData(nameIndex, newName)) {
                stadiumModel->submitAll(); // Push to project1.db
                // emit notifyStatus("Campus updated successfully!");
            } else {
                qDebug() << "Update failed:" << stadiumModel->lastError().text();
            }
        }
        qDebug() << "Update successful:" << stadiumModel->lastError().text();
    });
}

void DashboardPage::linkSouvenirDB(const QSqlDatabase &db) {
    setupSouvenirModel(db);

    ui->souvenirTableView->setModel(souvenirModel);

    setupSouvenirTableFormatting();

    // behavior for displaying souvenirs of the selected stadium
    setupSouvenirFiltering();
}
void DashboardPage::setupSouvenirModel(const QSqlDatabase &db) {
    // create model
    souvenirModel = new QSqlTableModel(this, db);
    souvenirModel->setTable("souvenirs");
    souvenirModel->setFilter("stadium_id = -1");

    if (!souvenirModel->select()) {
        qDebug() << "Model Error:" << souvenirModel->lastError().text();
    }

    if (!APP->databaseManager()->isStadiumModuleAvailable()) {
        qDebug() << "Data is not loaded yet!";
    }

    souvenirModel->setEditStrategy(QSqlTableModel::OnFieldChange);
}
void DashboardPage::setupSouvenirFiltering() {
    connect(ui->stadiumList, &QListView::clicked, this, [this](const QModelIndex &index) {
        // 1. Get the record from the STADIUM model
        QSqlRecord record = stadiumModel->record(index.row());

        // 2. Get the ID (Make sure this matches "stadium_id" in database_manager.cpp)
        int stadiumId = record.value("stadium_id").toInt();

        // 3. Apply the filter to the SOUVENIR model
        // This tells SQL: "SELECT * FROM souvenirs WHERE stadium_id = X"
        souvenirModel->setFilter(QString("stadium_id = %1").arg(stadiumId));

        // 4. Refresh the data
        souvenirModel->select();
    });
}
void DashboardPage::setupSouvenirTableFormatting() {
    // hide the souvenir and stadium id
    ui->souvenirTableView->setColumnHidden(0, true);
    ui->souvenirTableView->setColumnHidden(1, true);

    // rename headers for each column
    souvenirModel->setHeaderData(2, Qt::Horizontal, "Souvenir Name");
    souvenirModel->setHeaderData(3, Qt::Horizontal, "Price ($)");

    QHeaderView *header = ui->souvenirTableView->horizontalHeader();
    header->setSectionResizeMode(2, QHeaderView::Stretch);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
}


void DashboardPage::on_addSouvenirButton_clicked()
{
    auto stadiumIndex = ui->stadiumList->currentIndex();

    if (stadiumIndex.isValid()) {
        QString stadiumName = stadiumIndex.data().toString();

        auto stadium = APP->stadiumRepository()->getStadiumByStadiumName(stadiumName);

        newSouvenirPopup dialog(this);

        if (dialog.exec() == QDialog::Accepted)
        {
            QString name = dialog.getName();
            double price = dialog.getPrice();

            Souvenir item;

            item.owner_stadium_id = stadium->stadium_id;
            item.name = name;
            item.price = price;

            APP->souvenirRepository()->addSouvenir(stadium->stadium_id, item);

            qDebug() << name << price;

            souvenirModel->select();
        }
    }

    return;
}
void DashboardPage::on_removeSouvenirButton_clicked()
{
    QModelIndex index = ui->souvenirTableView->currentIndex();

    if (!index.isValid())
        return; // nothing selected

    int row = index.row();

    int id = souvenirModel->data(souvenirModel->index(row, 0)).toInt();

    APP->souvenirRepository()->deleteSouvenir(id);

    souvenirModel->select();
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
    {
        refreshConnections();
        if (stadiumModel != nullptr)
            stadiumModel->select();
        if (souvenirModel != nullptr)
            souvenirModel->select();
    }

    QMessageBox message_box(this);
    message_box.setWindowTitle("Import Files");
    message_box.setIcon(imported_anything ? QMessageBox::Information : QMessageBox::Warning);
    message_box.setText(result_lines.isEmpty()
                            ? "No importable files were selected."
                            : result_lines.join('\n'));
    message_box.exec();
}

void DashboardPage::refreshConnections() {
    // Get the NEW database handle after the reset
    const QSqlDatabase db = APP->databaseManager()->getDatabaseObj();

    // Delete old models if they exist to avoid holding dead connections
    if (stadiumModel) {
        stadiumModel->deleteLater();
        stadiumModel = nullptr;
    }
    if (souvenirModel) {
        souvenirModel->deleteLater();
        souvenirModel = nullptr;
    }
    if (distanceModel) {
        distanceModel->deleteLater();
        distanceModel = nullptr;
    }

    // Re-link with the fresh DB handle
    linkStadiumDB(db);
    linkSouvenirDB(db);
    linkDistanceDB(db);
}

void DashboardPage::on_removeStadiumButton_clicked()
{
    QModelIndex index = ui->stadiumList->currentIndex();

    if (!index.isValid())
        return; // nothing selected

    int row = index.row();

    int id = stadiumModel->data(stadiumModel->index(row, 0)).toInt();

    APP->stadiumRepository()->deleteStadium(id);

    stadiumModel->select();
}
void DashboardPage::on_addStadiumButton_clicked() {
     StadiumAdding dialog(this);
     if (dialog.exec() == QDialog::Accepted) {
         Stadium newStadium = dialog.getFormData();

         /*qDebug() << */APP->stadiumRepository()->addStadium(newStadium);

         // qDebug() << newStadium.league;

         stadiumModel->select();
     }
}

void DashboardPage::setupComboBox() {
     // Add leagues
     ui->leagueComboBox->addItem("National");
     ui->leagueComboBox->addItem("American");

     // Add ballpark typology
     ui->typologyComboBox->addItem("Retro Modern");
     ui->typologyComboBox->addItem("Retro Classic");
     ui->typologyComboBox->addItem("Jewel Box");
     ui->typologyComboBox->addItem("Modern");
     ui->typologyComboBox->addItem("Contemporary");
     ui->typologyComboBox->addItem("Multipurpose");

     // Add playing surfaces
     ui->surfaceComboBox->addItem("Grass");
     ui->surfaceComboBox->addItem("AstroTurf GameDay Grass");
     ui->surfaceComboBox->addItem("AstroTurf GameDay Grass 3D");

     // Add roof type
     ui->roofComboBox->addItem("Open");
     ui->roofComboBox->addItem("Retractable");
}
void DashboardPage::setupValidators() {
    // Regex for Names and Locations
    QRegularExpression nameRegex("^[a-zA-Z\\s\\-–'.]{1,100}$");
    QRegularExpression locRegex("^[a-zA-Z\\s,\\.\\-]{1,1000}$");

    QRegularExpressionValidator *nameValidator = new QRegularExpressionValidator(nameRegex, this);
    QRegularExpressionValidator *locValidator = new QRegularExpressionValidator(locRegex, this);

    // Assign to widgets
    ui->stadiumNameLineEdit->setValidator(nameValidator);
    ui->teamNameLineEdit->setValidator(nameValidator);
    ui->locationLineEdit->setValidator(locValidator);

    // SpinBox Ranges
    ui->seatingCapacitySpinBox->setRange(0, 200000);
    ui->dateOpenedSpinBox->setRange(1800, 2100);

    // Unified Styling Logic
    QList<QLineEdit*> lineEdits = {ui->stadiumNameLineEdit, ui->teamNameLineEdit, ui->locationLineEdit};
    for (QLineEdit* edit : lineEdits) {
        connect(edit, &QLineEdit::textChanged, [edit]() {
            if (edit->hasAcceptableInput() || edit->text().isEmpty()) {
                edit->setStyleSheet("");
            } else {
                edit->setStyleSheet("border: 1px solid red; background-color: #FFF0F0;");
            }
        });
    }
}
void DashboardPage::setupDetailsPanel() {
    ui->centerFieldSpinBox->setMaximum(1000000); // Sets the max to 1,000,000

    // 1. SYNC FROM LIST TO WIDGETS
    connect(ui->stadiumList, &QListView::clicked, this, [this](const QModelIndex &index) {
        QSqlRecord record = stadiumModel->record(index.row());

        // Sync LineEdits
        ui->stadiumNameLineEdit->setText(record.value("stadium_name").toString());
        ui->teamNameLineEdit->setText(record.value("team_name").toString());
        ui->locationLineEdit->setText(record.value("location").toString());

        // Sync ComboBoxes (Sets by string match)
        ui->leagueComboBox->setCurrentText(record.value("league").toString());
        ui->typologyComboBox->setCurrentText(record.value("ballpark_typology").toString());
        ui->surfaceComboBox->setCurrentText(record.value("playing_surface").toString());
        ui->roofComboBox->setCurrentText(record.value("roof_type").toString());

        // Sync SpinBoxes
        ui->seatingCapacitySpinBox->setValue(record.value("seating_capacity").toInt());
        ui->dateOpenedSpinBox->setValue(record.value("date_opened").toInt());
        ui->centerFieldSpinBox->setValue(record.value("distance_to_center_field_ft").toInt());
    });

    // 2. SYNC FROM WIDGETS TO DATABASE (Update on Enter/Change)

    // Example: Team Name Update (Column 1)
    connect(ui->teamNameLineEdit, &QLineEdit::returnPressed, this, [this]() {
        updateField(1, ui->teamNameLineEdit->text());
    });

    // Example: Location Update (Column 4)
    connect(ui->locationLineEdit, &QLineEdit::returnPressed, this, [this]() {
        updateField(4, ui->locationLineEdit->text());
    });

    // Example: League Update (ComboBox)
    connect(ui->leagueComboBox, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        updateField(6, text);
    });
    // Example: Surface Update (ComboBox)
    connect(ui->surfaceComboBox, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        updateField(5, text);
    });
    // Example: topology Update (ComboBox)
    connect(ui->typologyComboBox, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        updateField(10, text);
    });
    // Example: roof type Update (ComboBox)
    connect(ui->roofComboBox, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        updateField(11, text);
    });


    // SPINBOXES
    // Example: Capacity Update (SpinBox)
    void (QSpinBox::*valueChanged)(int) = &QSpinBox::valueChanged;
    connect(ui->seatingCapacitySpinBox, valueChanged, this, [this](int value) {
        updateField(3, value);
    });
    // Example: Date Opened Update (SpinBox)
    connect(ui->dateOpenedSpinBox, valueChanged, this, [this](int value) {
        updateField(7, value);
    });
    // Example: Centerfield Update (SpinBox)
    connect(ui->centerFieldSpinBox, valueChanged, this, [this](int value) {
        updateField(8, value);
    });
}
void DashboardPage::updateField(int columnIdx, const QVariant &value) {
    QModelIndex currentIndex = ui->stadiumList->currentIndex();
    if (currentIndex.isValid()) {
        QModelIndex targetIndex = stadiumModel->index(currentIndex.row(), columnIdx);
        if (stadiumModel->setData(targetIndex, value)) {
            stadiumModel->submitAll();
            qDebug() << "Column" << columnIdx << "updated successfully.";
        } else {
            qDebug() << "Update failed:" << stadiumModel->lastError().text();
        }
    }
}

void DashboardPage::linkDistanceDB(const QSqlDatabase &db) {
    setupDistanceModel(db);

    // Assuming your UI has a QTableView named distanceTableView
    ui->distancesTableView->setModel(distanceModel);

    setupDistanceTableFormatting();
    setupDistanceFiltering();
}

void DashboardPage::setupDistanceModel(const QSqlDatabase &db) {
    distanceModel = new QSqlTableModel(this, db);
    distanceModel->setTable("stadium_distances"); // Matches the table name in your DB schema

    // Set filter to empty initially or show all
    distanceModel->setFilter("stadium_a_id = ''");

    if (!distanceModel->select()) {
        qDebug() << "Distance Model Error:" << distanceModel->lastError().text();
    }

    distanceModel->setEditStrategy(QSqlTableModel::OnFieldChange);
}

void DashboardPage::setupDistanceTableFormatting() {
    // Hide IDs if they exist in the distances table (e.g., column 0)
    ui->distancesTableView->setColumnHidden(0, true);

    ui->distancesTableView->setItemDelegateForColumn(1, new StadiumNameDelegate(this));

    distanceModel->setHeaderData(0, Qt::Horizontal, "From Stadium");
    distanceModel->setHeaderData(1, Qt::Horizontal, "To Stadium");
    distanceModel->setHeaderData(2, Qt::Horizontal, "Distance (mi)");

    QHeaderView *header = ui->distancesTableView->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
}

void DashboardPage::setupDistanceFiltering() {
    // When a stadium is clicked, show all distances originating FROM that stadium
    connect(ui->stadiumList, &QListView::clicked, this, [this](const QModelIndex &index) {
        QSqlRecord record = stadiumModel->record(index.row());

        // Use the stadium name to filter (matches 'originated_stadium' column)
        int stadiumID = APP->stadiumRepository()->getStadiumByStadiumName(record.value("stadium_name").toString())->stadium_id;

        distanceModel->setFilter(QString("stadium_a_id = '%1'").arg(stadiumID));
        distanceModel->select();
    });
}


void DashboardPage::on_removeDistanceButton_clicked()
{
    // 1. Get the current selection from the table view
    QModelIndex index = ui->distancesTableView->currentIndex();
    if (!index.isValid()) {
        return; // Nothing selected
    }

    int row = index.row();

    // 2. Access the raw record from the model
    // This ignores the Delegate and gives you the actual database values
    QSqlRecord record = distanceModel->record(row);

    // 3. Extract the IDs using your database column names
    int originId = record.value("stadium_a_id").toInt();
    int destId = record.value("stadium_b_id").toInt();
    // 4. Call your repository function
    APP->distanceRepository()->removeDistanceBetweenStadiums(originId, destId);

    // 5. Refresh the UI
    distanceModel->select();
}

void DashboardPage::on_addDistanceButton_clicked() {
    QModelIndex stadiumIdx = ui->stadiumList->currentIndex();
    if (!stadiumIdx.isValid()) {
        QMessageBox::warning(this, "Selection Required", "Please select an originating stadium from the list first.");
        return;
    }

    // 1. Get the origin stadium details
    QSqlRecord originRecord = stadiumModel->record(stadiumIdx.row());
    int originId = originRecord.value("stadium_id").toInt();
    QString originName = originRecord.value("stadium_name").toString();

    // 2. Create the Popup Dialog on the stack
    QDialog dialog(this);
    dialog.setWindowTitle("Add New Distance from " + originName);

    QFormLayout form(&dialog);
    QComboBox* stadiumCombo = new QComboBox(&dialog);
    QLineEdit* distanceEdit = new QLineEdit(&dialog);
    distanceEdit->setValidator(new QIntValidator(1, 9999, &dialog));
    distanceEdit->setPlaceholderText("Miles");

    // 3. Identify existing destinations to exclude them (so we don't add duplicates)
    QSet<int> existingDestIds;
    for (int i = 0; i < distanceModel->rowCount(); ++i) {
        existingDestIds.insert(distanceModel->record(i).value("stadium_b_id").toInt());
    }

    // 4. Populate the ComboBox with valid targets
    // We iterate through the stadiumModel to find stadiums that aren't 'Us' and aren't already linked
    for (int i = 0; i < stadiumModel->rowCount(); ++i) {
        QSqlRecord rec = stadiumModel->record(i);
        int targetId = rec.value("stadium_id").toInt();
        QString targetName = rec.value("stadium_name").toString();

        if (targetId != originId && !existingDestIds.contains(targetId)) {
            stadiumCombo->addItem(targetName, targetId); // Store ID in UserData
        }
    }

    if (stadiumCombo->count() == 0) {
        QMessageBox::information(this, "No Options", "All possible stadium connections already exist.");
        return;
    }

    form.addRow("To Stadium:", stadiumCombo);
    form.addRow("Distance (mi):", distanceEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // 5. Execute and Save
    if (dialog.exec() == QDialog::Accepted) {
        int destinationId = stadiumCombo->currentData().toInt();
        int distance = distanceEdit->text().toInt();

        if (distance <= 0) {
            QMessageBox::warning(this, "Invalid Input", "Please enter a valid distance.");
            return;
        }

        // Call your repository to save the distance
        bool success = APP->distanceRepository()->addDistanceBetweenStadiums(originId, destinationId, distance);

        if (success) {
            distanceModel->select(); // Refresh the table view
            qDebug() << "Distance added successfully between" << originId << "and" << destinationId;
        } else {
            QMessageBox::critical(this, "Database Error", "Failed to insert distance into the database.");
        }
    }
}