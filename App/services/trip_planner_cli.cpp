#include <QCoreApplication>
#include <QDebug>
#include <iostream>

#include "../application.h"
#include "../services/trip_planner.h"

using std::cin;
using std::cout;
using std::endl;

// ===============================
// Print Trip safely
// ===============================
void printTrip(Trip* trip)
{
    if (!trip)
    {
        qDebug() << "Trip is NULL!";
        return;
    }

    const TripResult& result = trip->getResult();

    qDebug() << "\n==== TRIP RESULT ====";

    for (const auto& stadium : result.stadiums)
    {
        qDebug() << "Stadium ID:" << stadium.stadium_id;
    }

    qDebug() << "Total distance:" << result.total_distance;
}

// ===============================
// MAIN CLI
// ===============================
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "===== CLI TEST START =====";

    // Get Application singleton
    Application* core = Application::instance();

    if (!core)
    {
        qDebug() << "Application instance is NULL!";
        return -1;
    }

    if (!core->init())
    {
        qDebug() << "Application init failed:" << core->lastError();
        return -1;
    }

    qDebug() << "TripPlanner available:" << core->isTripPlannerAvailable();
    qDebug() << "Warning:" << core->lastWarning();

    // IMPORTANT: use getTripPlanner()
    TripPlanner* planner = core->getTripPlanner();

    if (!planner)
    {
        qDebug() << "TripPlanner is NULL!";
        return -1;
    }

    int choice = 0;

    while (true)
    {
        cout << "\n==== BASEBALL TRIP PLANNER CLI ====\n";
        cout << "1. List stadium IDs\n";
        cout << "2. Shortest path\n";
        cout << "3. Visit all (Nearest)\n";
        cout << "4. DFS traversal\n";
        cout << "5. BFS traversal\n";
        cout << "6. Exit\n";
        cout << "Choice: ";

        cin >> choice;

        if (choice == 1)
        {
            auto ids = planner->getAllStadiumIds();

            cout << "Stadium IDs:\n";
            for (int id : ids)
                cout << id << endl;
        }
        else if (choice == 2)
        {
            int start, target;

            cout << "Start ID: ";
            cin >> start;

            cout << "Target ID: ";
            cin >> target;

            if (planner->planShortestTripToTarget(start, target))
                printTrip(planner->getCurrentTrip());
            else
                qDebug() << "Failed shortest path";
        }
        else if (choice == 3)
        {
            int start;

            cout << "Start ID: ";
            cin >> start;

            if (planner->planVisitAllByNearestFrom(start))
                printTrip(planner->getCurrentTrip());
            else
                qDebug() << "Failed visit-all";
        }
        else if (choice == 4)
        {
            int start;

            cout << "Start ID: ";
            cin >> start;

            if (planner->generateDFSResultFrom(start))
                printTrip(planner->getCurrentTrip());
            else
                qDebug() << "DFS failed";
        }
        else if (choice == 5)
        {
            int start;

            cout << "Start ID: ";
            cin >> start;

            if (planner->generateBFSResultFrom(start))
                printTrip(planner->getCurrentTrip());
            else
                qDebug() << "BFS failed";
        }
        else if (choice == 6)
        {
            cout << "Exiting...\n";
            break;
        }
        else
        {
            cout << "Invalid choice\n";
        }
    }

    return 0;
}