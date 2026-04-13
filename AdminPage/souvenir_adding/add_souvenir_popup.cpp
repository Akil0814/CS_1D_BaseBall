//
// Created by Erfan Tavassoli on 4/8/26.
//

// You may need to build the project (run Qt uic code generator) to get "ui_add_souvenir_popup.h" resolved

#include "add_souvenir_popup.h"
#include "ui_add_souvenir_popup.h"


add_souvenir_popup::add_souvenir_popup(QWidget *parent) :
    QDialog(parent), ui(new Ui::add_souvenir_popup) {
    ui->setupUi(this);
}

add_souvenir_popup::~add_souvenir_popup() {
    delete ui;
}
