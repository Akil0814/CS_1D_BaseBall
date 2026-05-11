
#include "stadium_adding.h"

#include <QMessageBox>
#include "ui_stadium_adding.h"
#include <string>

#include "App/application.h"
using namespace std;

StadiumAdding::StadiumAdding(QWidget *parent)
    : QDialog(parent), ui(new Ui::StadiumAdding) {
    ui->setupUi(this);

    setupValidators();

    setupComboBox();
}

StadiumAdding::~StadiumAdding() { delete ui; }

void StadiumAdding::setupComboBox() {
    // Add leagues
    ui->leagueComboBox->addItem("National");
    ui->leagueComboBox->addItem("American");

    // Add ballpark typology
    ui->typologyComboBox->addItem("Retro Modern");
    ui->typologyComboBox->addItem("Retro Classic");
    ui->typologyComboBox->addItem("Jewel Box");
    ui->typologyComboBox->addItem("Modern");
    ui->typologyComboBox->addItem("Contemporary");
    ui->typologyComboBox->addItem("Multipurpose");

    // Add playing surfaces
    ui->surfaceComboBox->addItem("Grass");
    ui->surfaceComboBox->addItem("AstroTurf GameDay Grass");
    ui->surfaceComboBox->addItem("AstroTurf GameDay Grass 3D");

    // Add roof type
    ui->roofComboBox->addItem("Open");
    ui->roofComboBox->addItem("Retractable");
}

void StadiumAdding::setupValidators() {
    // 1. Validate Stadium and Team Names (Letters, spaces, and hyphens only, max 100 chars)
    // Regex: Allow a-z, A-Z, spaces, and hyphens.
    QRegularExpression nameRegex("^[a-zA-Z\\s-]{1,100}$");
    QRegularExpressionValidator *nameValidator = new QRegularExpressionValidator(nameRegex, this);

    ui->stadiumNameLineEdit->setValidator(nameValidator);
    ui->teamNameLineEdit->setValidator(nameValidator);

    // 2. Validate Location (Letters, spaces, commas for "City, State")
    QRegularExpression locRegex("^[a-zA-Z\\s,]{1,100}$");
    QRegularExpressionValidator *locValidator = new QRegularExpressionValidator(locRegex, this);

    ui->locationLineEdit->setValidator(locValidator);

    // 3. Adjust SpinBox Ranges (The "built-in" validation for numbers)
    // Seating Capacity: 0 to 200,000
    ui->seatingCapacitySpinBox->setRange(0, 200000);

    // Date Opened: From the first ballpark (1800s) to the near future
    ui->dateOpenedSpinBox->setRange(1800, 2100);

    QList<QLineEdit*> lineEdits = {ui->stadiumNameLineEdit, ui->teamNameLineEdit, ui->locationLineEdit};

    for (QLineEdit* edit : lineEdits) {
        connect(edit, &QLineEdit::textChanged, [edit]() {
            // If the text is partially valid or empty, keep it red.
            // If it's fully 'Acceptable' by the regex, clear the style.
            if (edit->hasAcceptableInput()) {
                edit->setStyleSheet("");
            } else {
                edit->setStyleSheet("QLineEdit { border: 1px solid red; background-color: #FFF0F0; }");
            }
        });
    }
}

Stadium StadiumAdding::getFormData() const {
    Stadium data;

    // LineEdits
    data.stadium_name = ui->stadiumNameLineEdit->text().trimmed();
    data.team_name = ui->teamNameLineEdit->text().trimmed();
    data.location = ui->locationLineEdit->text().trimmed();

    // SpinBoxes
    data.seating_capacity = ui->seatingCapacitySpinBox->value();
    data.date_opened = ui->dateOpenedSpinBox->value();
    data.distance_to_center_field_ft = ui->centerFieldSpinBox->value();
    data.distance_to_center_field_raw = QString::number(ui->centerFieldSpinBox->value());

    // ComboBoxes
    data.league = ui->leagueComboBox->currentText();
    data.playing_surface = ui->surfaceComboBox->currentText();
    data.ballpark_typology = ui->typologyComboBox->currentText();
    data.roof_type = ui->roofComboBox->currentText();

    return data;
}

void StadiumAdding::on_buttonBox_accepted() {
    bool isStadValid = ui->stadiumNameLineEdit->hasAcceptableInput();
    bool isTeamValid = ui->teamNameLineEdit->hasAcceptableInput();
    bool isLocValid  = ui->locationLineEdit->hasAcceptableInput();

    if (!isStadValid || !isTeamValid || !isLocValid) {
        // Trigger the red outline immediately for all invalid fields
        // (This helps if the user hasn't typed anything yet)
        if (!isStadValid) ui->stadiumNameLineEdit->setStyleSheet("border: 1px solid red;");
        if (!isTeamValid) ui->teamNameLineEdit->setStyleSheet("border: 1px solid red;");
        if (!isLocValid)  ui->locationLineEdit->setStyleSheet("border: 1px solid red;");

        QMessageBox::critical(this, "Input Error",
            "Some fields are invalid or empty. Please check the highlighted red boxes.");

        // This is the "Passive" stop: we don't call accept(), so the dialog stays open
        return;
    }

    // If we reach here, everything is valid!
    this->accept();
}

