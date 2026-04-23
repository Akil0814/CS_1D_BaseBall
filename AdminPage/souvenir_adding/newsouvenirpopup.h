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

    QString getName() const;
    double getPrice() const;
protected:
    void accept() override;

private:
    Ui::newSouvenirPopup *ui;
    QString m_name;
    double m_price = 0.0;
};

#endif // NEWSOUVENIRPOPUP_H
