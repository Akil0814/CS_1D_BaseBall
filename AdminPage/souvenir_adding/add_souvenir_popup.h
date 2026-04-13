//
// Created by Erfan Tavassoli on 4/8/26.
//

#ifndef GITIGNORE_ADD_SOUVENIR_POPUP_H
#define GITIGNORE_ADD_SOUVENIR_POPUP_H

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class add_souvenir_popup; }
QT_END_NAMESPACE

class add_souvenir_popup : public QDialog {
Q_OBJECT

public:
    explicit add_souvenir_popup(QWidget *parent = nullptr);
    ~add_souvenir_popup() override;

private:
    Ui::add_souvenir_popup *ui;
};


#endif //GITIGNORE_ADD_SOUVENIR_POPUP_H
