#ifndef STADIUM_ADDING_H
#define STADIUM_ADDING_H

#include <QDialog>

#include "data_types.h"

namespace Ui {
class StadiumAdding;
}

class StadiumAdding : public QDialog {
  Q_OBJECT

public:
  explicit StadiumAdding(QWidget *parent = nullptr);
  ~StadiumAdding();
  Stadium getFormData() const;


private slots:
  void setupComboBox();
  void on_buttonBox_accepted();

private:
    Ui::StadiumAdding *ui;
    void setupValidators();
};

#endif // STADIUM_ADDING_H
