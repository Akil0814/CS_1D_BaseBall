//
// Created by Erfan Tavassoli on 4/13/26.
//

// You may need to build the project (run Qt uic code generator) to get "ui_main_page.h" resolved

#include "main_page.h"
#include "ui_main_page.h"
#include "browse_window.h"

class BrowseWindow;

main_page::main_page(QWidget *parent) :
    QWidget(parent), ui(new Ui::main_page) {
    ui->setupUi(this);

    connect(ui->btnBrowse, &QPushButton::clicked, this, [this]() {
        auto *browseWindow = new BrowseWindow(this);
        browseWindow->show();
    });
}

main_page::~main_page() {
    delete ui;
}
