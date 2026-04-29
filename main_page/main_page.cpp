#include "main_page.h"
#include "ui_main_page.h"
#include "App/application.h"

#include <QPixmap>

main_page::main_page(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::main_page)
{
    ui->setupUi(this);

    // dark navy background
    // ui->mainContainer->setStyleSheet(
    //     "QWidget#mainContainer {"
    //     "background-color: #0A2A66;"
    //     "}"
    //     );

    // show MLB logo
    QString path = APP->assetsDir();
    QPixmap logo(path+"/images/mlb.png");
    ui->lblLogo->setPixmap(
        logo.scaled(420, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation)
        );
    ui->lblLogo->setAlignment(Qt::AlignCenter);
}

main_page::~main_page()
{
    delete ui;
}
