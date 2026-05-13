#ifndef END_TRIP_SUMMARY_H
#define END_TRIP_SUMMARY_H

#include <QStandardItemModel>
#include <QWidget>

#include "application.h"

class QStringListModel;

namespace Ui {
class end_trip_summary;
}

class end_trip_summary : public QWidget {
  Q_OBJECT

public:
  explicit end_trip_summary(QWidget *parent = nullptr);
  ~end_trip_summary();

private slots:
  void setupStops();
  void setupPurchases();

  QString resolveStadiumName(int stadium_id) const;

private:
  Ui::end_trip_summary *ui;
  QStringListModel* stopsModel;
  QStandardItemModel* cartModel;
  Trip* _current_trip = APP->tripPlanner()->getCurrentTrip();

};

#endif // END_TRIP_SUMMARY_H
