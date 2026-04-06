//
// Created by Erfan Tavassoli on 4/6/26.
//

#ifndef GITIGNORE_ADMINPAGE_H
#define GITIGNORE_ADMINPAGE_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class AdminPage; }
QT_END_NAMESPACE

class AdminPage : public QWidget {
Q_OBJECT

public:
    explicit AdminPage(QWidget *parent = nullptr);
    ~AdminPage() override;

private:
    Ui::AdminPage *ui;
};


#endif //GITIGNORE_ADMINPAGE_H
