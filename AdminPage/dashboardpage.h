//
// Created by Erfan Tavassoli on 4/6/26.
//

#ifndef GITIGNORE_DASHBOARDPAGE_H
#define GITIGNORE_DASHBOARDPAGE_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class DashboardPage; }
QT_END_NAMESPACE

class DashboardPage : public QWidget {
Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage() override;

private:
    Ui::DashboardPage *ui;
};


#endif //GITIGNORE_DASHBOARDPAGE_H
