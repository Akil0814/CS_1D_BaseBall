#include "newsouvenirpopup.h"
#include "ui_newsouvenirpopup.h"
#include <QMessageBox>
#include <QRegularExpression>

newSouvenirPopup::newSouvenirPopup(QWidget *parent)
    : QDialog(parent), ui(new Ui::newSouvenirPopup) {
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &QDialog::accept);

    connect(ui->buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);
}

newSouvenirPopup::~newSouvenirPopup() { delete ui; }

QString newSouvenirPopup::getName() const
{
    return m_name;
}

double newSouvenirPopup::getPrice() const
{
    return m_price;
}

void newSouvenirPopup::accept()
{
    QString name = ui->souvenirNameLineEdit->text().trimmed();
    QString priceText = ui->souvenirPriceLineEdit->text().trimmed();

    // Check empty fields
    if (name.isEmpty() || priceText.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input",
                             "Name and Price cannot be empty.");
        return;
    }

    // Name validation (letters + spaces only)
    QRegularExpression nameRegex("^[A-Za-z\\s]+$");
    if (!nameRegex.match(name).hasMatch()) {
        QMessageBox::warning(this, "Invalid Name",
                             "Name must contain only letters and spaces.");
        return;
    }

    // Price validation (must be a valid double)
    bool ok = false;
    double price = priceText.toDouble(&ok);

    if (!ok || price < 0) {
        QMessageBox::warning(this, "Invalid Price",
                             "Price must be a valid positive number.");
        return;
    }

    // If valid → store parsed value if you want
    m_name = name;
    m_price = price;

    QDialog::accept(); // close dialog
}
