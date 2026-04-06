//
// Created by Erfan Tavassoli on 4/6/26.
//

#ifndef GITIGNORE_LOGINPAGE_H
#define GITIGNORE_LOGINPAGE_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class LoginPage; }
QT_END_NAMESPACE

class LoginPage : public QWidget {
Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);
    ~LoginPage() override;

private:
    Ui::LoginPage *ui;

signals:
    void loginAccepted();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected() const;
    // void on_
};


#endif //GITIGNORE_LOGINPAGE_H
