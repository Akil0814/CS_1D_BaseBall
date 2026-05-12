#include <QCoreApplication>
#include <iostream>

#include "../application.h"
#include "../services/trip_planner.h"

using std::cin;
using std::cerr;
using std::cout;
using std::endl;

// ===============================
// Print Trip safely
// ===============================
void printTrip(Trip* trip)
{
    if (!trip)
    {
        cerr << "Trip is NULL!" << endl;
        return;
    }

    const TripResult& result = trip->getResult();

    cout << "\n==== TRIP RESULT ====\n";

    for (size_t index = 0; index < result.stadiums.size(); ++index)
    {
        const Stadium& stadium = result.stadiums[index];
        const bool is_transit =
            index < result.transit_flags.size() && result.transit_flags[index];

        cout << index + 1 << ". "
             << stadium.stadium_name.toStdString()
             << " [ID: " << stadium.stadium_id << "]";

        if (is_transit)
            cout << " (Transit)";

        cout << '\n';
    }

    cout << "Total distance: " << result.total_distance << '\n';
}

// ===============================
// MAIN CLI
// ===============================
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    cout << "===== CLI TEST START =====\n";

    // Get Application singleton
    Application* core = Application::instance();

    if (!core)
    {
        cerr << "Application instance is NULL!" << endl;
        return -1;
    }

    if (!core->init())
    {
        cerr << "Application init failed: "
             << core->lastError().toStdString()
             << endl;
        return -1;
    }

    cout << "TripPlanner available: "
         << (core->isTripPlannerAvailable() ? "true" : "false")
         << '\n';

    if (!core->lastWarning().isEmpty())
        cout << "Warning: " << core->lastWarning().toStdString() << '\n';

    // IMPORTANT: use getTripPlanner()
    TripPlanner* planner = core->getTripPlanner();

    if (!planner)
    {
        cerr << "TripPlanner is NULL!" << endl;
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
                cerr << "Failed shortest path" << endl;
        }
        else if (choice == 3)
        {
            int start;

            cout << "Start ID: ";
            cin >> start;

            if (planner->planVisitAllByNearestFrom(start))
                printTrip(planner->getCurrentTrip());
            else
                cerr << "Failed visit-all" << endl;
        }
        else if (choice == 4)
        {
            int start;

            cout << "Start ID: ";
            cin >> start;

            if (planner->generateDFSResultFrom(start))
                printTrip(planner->getCurrentTrip());
            else
                cerr << "DFS failed" << endl;
        }
        else if (choice == 5)
        {
            int start;

            cout << "Start ID: ";
            cin >> start;

            if (planner->generateBFSResultFrom(start))
                printTrip(planner->getCurrentTrip());
            else
                cerr << "BFS failed" << endl;
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
