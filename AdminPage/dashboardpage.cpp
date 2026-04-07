//
// Created by Erfan Tavassoli on 4/6/26.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DashboardPage.h" resolved

#include "dashboardpage.h"

#include <QSqlError>
#include <QSqlRecord>
#include "ui_DashboardPage.h"
#include "App/application.h"
#include <QSqlTableModel>


DashboardPage::DashboardPage(QWidget *parent) :
    QWidget(parent), ui(new Ui::DashboardPage) {
    ui->setupUi(this);

    const QSqlDatabase db = APP->databaseManager()->getDatabaseObj();

    linkStadiumDB(db);
    linkSouvenirDB(db);
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