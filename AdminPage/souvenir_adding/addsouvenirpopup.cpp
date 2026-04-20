#include "addsouvenirpopup.h"
#include "ui_addsouvenirpopup.h"

AddSouvenirPopup::AddSouvenirPopup(QWidget *parent)
    : QDialog(parent), ui(new Ui::AddSouvenirPopup) {
  ui->setupUi(this);
}

AddSouvenirPopup::~AddSouvenirPopup() { delete ui; }
