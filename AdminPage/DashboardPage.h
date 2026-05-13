//
// Created by Erfan Tavassoli on 4/6/26.
//

#ifndef GITIGNORE_DASHBOARDPAGE_H
#define GITIGNORE_DASHBOARDPAGE_H

#include <QSqlTableModel>
#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class DashboardPage; }
QT_END_NAMESPACE

class DashboardPage : public QWidget {
Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage() override;

    void linkStadiumDB(const QSqlDatabase &db);
    void setupStadiumModel(const QSqlDatabase &db);
    void setupStadiumNameField();

    void linkSouvenirDB(const QSqlDatabase &db);
    void setupSouvenirModel(const QSqlDatabase &db);
    void setupSouvenirFiltering();
    void setupSouvenirTableFormatting();

private slots:
    void on_addSouvenirButton_clicked();
    void on_removeSouvenirButton_clicked();
    void on_removeDistanceButton_clicked();
    void on_addDistanceButton_clicked();
    void on_uploadFile_clicked();

    void refreshConnections();
    void on_removeStadiumButton_clicked();
    void on_addStadiumButton_clicked();
    void setupComboBox();
    void setupValidators();
    void setupDetailsPanel();
    void updateField(int columnIdx, const QVariant &value);
    void linkDistanceDB(const QSqlDatabase &db);
    void setupDistanceModel(const QSqlDatabase &db);
    void setupDistanceTableFormatting();
    void setupDistanceFiltering();


private:
    Ui::DashboardPage *ui;
    QSqlTableModel* stadiumModel{};
    QSqlTableModel* souvenirModel{};
    QSqlTableModel* distanceModel{};
};


#endif //GITIGNORE_DASHBOARDPAGE_H
