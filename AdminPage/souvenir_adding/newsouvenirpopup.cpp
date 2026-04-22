#include "newsouvenirpopup.h"
#include "ui_newsouvenirpopup.h"

newSouvenirPopup::newSouvenirPopup(QWidget *parent)
    : QDialog(parent), ui(new Ui::newSouvenirPopup) {
  ui->setupUi(this);
}

newSouvenirPopup::~newSouvenirPopup() { delete ui; }
