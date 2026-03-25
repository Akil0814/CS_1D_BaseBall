#ifndef TRIP_PLANNER_H
#define TRIP_PLANNER_H

class Application;

class TripPlanner
{
    friend class Application;

public:


private:
    TripPlanner();

    TripPlanner(const TripPlanner& copy) = delete;
    TripPlanner& operator=(const TripPlanner& copy) = delete;
    TripPlanner(TripPlanner&& move) = delete;
    TripPlanner& operator=(TripPlanner&& move) = delete;
};

#endif // TRIP_PLANNER_H
