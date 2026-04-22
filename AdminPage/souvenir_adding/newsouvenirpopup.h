#ifndef NEWSOUVENIRPOPUP_H
#define NEWSOUVENIRPOPUP_H

#include <QDialog>

namespace Ui {
class newSouvenirPopup;
}

class newSouvenirPopup : public QDialog {
  Q_OBJECT

public:
  explicit newSouvenirPopup(QWidget *parent = nullptr);
  ~newSouvenirPopup();

private:
  Ui::newSouvenirPopup *ui;
};

#endif // NEWSOUVENIRPOPUP_H
