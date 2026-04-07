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

    linkStadiumDB();
}

DashboardPage::~DashboardPage() {
    delete ui;
}

void DashboardPage::linkStadiumDB() {
    setupStadiumModel();

    ui->stadiumList->setModel(stadiumModel);
    ui->stadiumList->setModelColumn(2);

    setupStadiumNameField();
}



void DashboardPage::setupStadiumModel() {
    QSqlDatabase db = APP->databaseManager()->getDatabaseObj();

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
    // 2. Handle the "Click" on the Campus List
    connect(ui->stadiumList, &QListView::clicked, this, [this](const QModelIndex &index) {
        // Get the full record for the row you just clicked
        QSqlRecord record = stadiumModel->record(index.row());

        // Update the LineEdit with the name
        QString name = record.value("stadium_name").toString(); // Use your actual column name
        ui->stadiumNameLineEdit->setText(name);

        // Filter the Souvenirs table by Campus ID
        // int campusId = record.value("campusId").toInt(); // Use your actual ID column name
        // souvenirModel->setFilter(QString("campusId = %1").arg(campusId));
        // souvenirModel->select();
    });

    // 3. Handle live-editing the name back to the DB
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