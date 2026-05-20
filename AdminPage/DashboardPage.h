//
// Created by Erfan Tavassoli on 4/6/26.
//

#ifndef GITIGNORE_DASHBOARDPAGE_H
#define GITIGNORE_DASHBOARDPAGE_H

#include "../data_types.h"

#include <QVariant>
#include <QWidget>

#include <optional>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class DashboardPage; }
QT_END_NAMESPACE

class QModelIndex;
class QStandardItemModel;

class DashboardPage : public QWidget {
Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage() override;

private slots:
    void on_addSouvenirButton_clicked();
    void on_removeSouvenirButton_clicked();
    void on_removeDistanceButton_clicked();
    void on_addDistanceButton_clicked();
    void on_uploadFile_clicked();

    void refreshConnections();
    void on_removeStadiumButton_clicked();
    void on_addStadiumButton_clicked();
    void setupComboBox();
    void setupValidators();
    void setupDetailsPanel();
    void updateField(int columnIdx, const QVariant &value);

private:
    void setupStadiumModel();
    void setupStadiumNameField();
    void setupSouvenirModel();
    void setupSouvenirTableFormatting();
    void setupDistanceModel();
    void setupDistanceTableFormatting();

    void populateStadiumModel(int selected_stadium_id = -1);
    void populateSouvenirModel(int stadium_id);
    void populateDistanceModel(int stadium_id);
    void populateDetailsPanel(const Stadium* stadium);
    void handleStadiumSelectionChanged(const QModelIndex& current_index);

    std::optional<int> currentStadiumId() const;
    std::optional<Stadium> currentStadium() const;
    QString stadiumDisplayText(const Stadium& stadium) const;
    QString stadiumNameById(int stadium_id) const;

private:
    Ui::DashboardPage *ui;
    QStandardItemModel* stadiumModel{};
    QStandardItemModel* souvenirModel{};
    QStandardItemModel* distanceModel{};

    std::vector<Stadium> _stadiums;
    std::vector<Souvenir> _souvenirs;
    std::vector<DistanceEdge> _distances;

    bool _is_loading_details = false;
    bool _is_populating_models = false;
};


#endif //GITIGNORE_DASHBOARDPAGE_H
