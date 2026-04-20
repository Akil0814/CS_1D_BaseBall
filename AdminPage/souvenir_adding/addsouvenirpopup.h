#ifndef ADDSOUVENIRPOPUP_H
#define ADDSOUVENIRPOPUP_H

#include <QDialog>

namespace Ui {
class AddSouvenirPopup;
}

class AddSouvenirPopup : public QDialog {
  Q_OBJECT

public:
  explicit AddSouvenirPopup(QWidget *parent = nullptr);
  ~AddSouvenirPopup();

private:
  Ui::AddSouvenirPopup *ui;
};

#endif // ADDSOUVENIRPOPUP_H
