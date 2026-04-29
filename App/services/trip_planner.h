// #ifndef TRIP_PLANNER_H
// #define TRIP_PLANNER_H
// #include <memory>
//
// #include "../model/trip.h"
//
// class Application;
//
// class TripPlanner
// {
//     friend class Application;
//
// public:
//
//     //bool planShortestTripToTarget(int start_stadium_id, int target_stadium_id);
//     //bool planCustomOrderedTrip(int start_stadium_id, const std::vector<int>& targets_in_order);
//     //bool planVisitAllByNearestFrom(int start_stadium_id);
//     //bool planCustomUnorderedEfficientTrip(int start_stadium_id, const std::vector<int>& selected_targets);
//
//     //bool generateMSTResult();
//     //bool generateDFSResultFrom(int start_stadium_id);
//     //bool generateBFSResultFrom(int start_stadium_id);
//
//     Trip* getCurrentTrip() const;
//
// private:
//     TripPlanner();
//
//     TripPlanner(const TripPlanner& copy) = delete;
//     TripPlanner& operator=(const TripPlanner& copy) = delete;
//     TripPlanner(TripPlanner&& move) = delete;
//     TripPlanner& operator=(TripPlanner&& move) = delete;
//
// private:
//     std::unique_ptr<Trip> _current_trip;
// };
//
// #endif // TRIP_PLANNER_H


#ifndef TRIP_PLANNER_H
#define TRIP_PLANNER_H

#include <memory>
#include <vector>

#include "../model/trip.h"
#include "../../data_types.h"
#include "../data_access/stadium_repository.h"

class TripPlanner
{
    friend class Application;

public:
    explicit TripPlanner(StadiumRepository& repo);

    [[nodiscard]] Trip* getCurrentTrip() const;


    bool planCustomOrderedTrip(int start_stadium_id,
                               const std::vector<int>& targets_in_order);

    bool planCustomUnorderedEfficientTrip(int start_stadium_id,
                                           const std::vector<int>& selected_targets);

    bool planShortestTripToTarget(int start_stadium_id, int target_stadium_id);

    bool planVisitAllByNearestFrom(int start_stadium_id);

    bool generateMSTResult();
    bool generateDFSResultFrom(int start_stadium_id);
    bool generateBFSResultFrom(int start_stadium_id);
    [[nodiscard]] std::vector<int> getAllStadiumIds() const;
    std::vector<int> getNeighbors(int id);

private:
    double getDistance(int from_id, int to_id) const;
    Stadium getStadiumById(int id);


private:
    std::unique_ptr<Trip> _current_trip;
    StadiumRepository& _repo;
};

#endif // TRIP_PLANNER_H