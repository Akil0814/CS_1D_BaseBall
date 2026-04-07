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

    void linkStadiumDB();

    void setupStadiumModel();

void setupStadiumNameField();

void linkSouvenirDB();

void setupSouvenirModel();

void setupSouvenirFiltering();

void setupSouvenirTableFormatting();

void linkStadiumNameField(QSqlTableModel* stadiumModel);

private:
    Ui::DashboardPage *ui;
    QSqlTableModel* stadiumModel;
    QSqlTableModel* souvenirModel;
};


#endif //GITIGNORE_DASHBOARDPAGE_H
