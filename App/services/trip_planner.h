#ifndef TRIP_PLANNER_H
#define TRIP_PLANNER_H

#include <memory>
#include <unordered_map>
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
    std::vector<int> getNeighbors(int id) const;

private:
    double getDistance(int from_id, int to_id) const;
    Stadium getStadiumById(int id) const;
    bool computeShortestPaths(
        int start_id,
        std::unordered_map<int, double>& distances,
        std::unordered_map<int, int>& previous
        ) const;
    bool buildShortestPath(
        int start_id,
        int target_id,
        const std::unordered_map<int, double>& distances,
        const std::unordered_map<int, int>& previous,
        std::vector<int>& path
        ) const;
    bool buildAnnotatedRouteFromVisitOrder(
        const std::vector<int>& visit_order,
        std::vector<int>& route,
        std::vector<bool>& transit_flags
        ) const;
    TripResult buildTripResultFromRoute(
        const std::vector<int>& route,
        const std::vector<bool>& transit_flags
        ) const;
    std::vector<int> getNeighborsSortedByDistance(int id) const;


private:
    std::unique_ptr<Trip> _current_trip;
    StadiumRepository& _repo;
};

#endif // TRIP_PLANNER_H
