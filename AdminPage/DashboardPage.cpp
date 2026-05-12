 //
// Created by Erfan Tavassoli on 4/6/26.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DashboardPage.h" resolved

#include "DashboardPage.h"

#include <QSqlError>
#include <QSqlRecord>
#include "ui_DashboardPage.h"
#include "App/application.h"
#include <QSqlTableModel>
#include <souvenir_adding/newsouvenirpopup.h>

#include "stadium_adding/stadium_adding.h"


DashboardPage::DashboardPage(QWidget *parent) :
QWidget(parent), ui(new Ui::DashboardPage) {
    ui->setupUi(this);

    const QSqlDatabase db = APP->databaseManager()->getDatabaseObj();

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

    // Re-link with the fresh DB handle
    linkStadiumDB(db);
    linkSouvenirDB(db);
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