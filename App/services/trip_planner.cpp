#include "trip_planner.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <limits>
#include <algorithm>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <functional>

// =====================================================
// Constructor
// =====================================================
TripPlanner::TripPlanner(StadiumRepository& repo)
    : _repo(repo),
      _current_trip(nullptr)
{
}

// =====================================================
// Getter
// =====================================================
Trip* TripPlanner::getCurrentTrip() const
{
    return _current_trip.get();
}

// =====================================================
// REAL distance lookup (DB-based)
// =====================================================
// double TripPlanner::getDistance(int from_id, int to_id) const
// {
//     if (from_id == to_id)
//         return 0.0;
//
//     QSqlDatabase db = _repo.getDatabaseManager().getDatabaseObj();
//     if (!db.isOpen())
//         return std::numeric_limits<double>::max();
//
//     int a = std::min(from_id, to_id);
//     int b = std::max(from_id, to_id);
//
//     QSqlQuery q(db);
//     q.prepare(R"(
//         SELECT distance_miles
//         FROM stadium_distances
//         WHERE stadium_a_id = ? AND stadium_b_id = ?
//     )");
//
//     q.addBindValue(a);
//     q.addBindValue(b);
//
//     if (!q.exec())
//         return std::numeric_limits<double>::max();
//
//     if (q.next())
//         return q.value(0).toDouble();
//
//     return std::numeric_limits<double>::max();
// }
double TripPlanner::getDistance(int from_id, int to_id) const
{
    if (from_id == to_id)
        return 0.0;

    QSqlDatabase db = _repo.getDatabaseManager().getDatabaseObj();

    if (!db.isOpen())
        return std::numeric_limits<double>::max();

    int a = std::min(from_id, to_id);
    int b = std::max(from_id, to_id);

    QSqlQuery q(db);
    q.prepare(R"(
        SELECT distance_miles
        FROM stadium_distances
        WHERE stadium_a_id = ? AND stadium_b_id = ?
    )");

    q.addBindValue(a);
    q.addBindValue(b);

    if (!q.exec())
        return std::numeric_limits<double>::max();

    if (q.next())
        return q.value(0).toDouble();

    return std::numeric_limits<double>::max();
}

// =====================================================
// Helper: load Stadium by ID
// =====================================================
Stadium TripPlanner::getStadiumById(int id) const
{
    auto s = _repo.getStadiumByID(id);
    if (s.has_value())
        return *s;

    return Stadium{};
}

bool TripPlanner::computeShortestPaths(
    int start_id,
    std::unordered_map<int, double>& distances,
    std::unordered_map<int, int>& previous
    ) const
{
    distances.clear();
    previous.clear();

    const auto all_ids = getAllStadiumIds();
    if (all_ids.empty())
        return false;

    if (std::find(all_ids.begin(), all_ids.end(), start_id) == all_ids.end())
        return false;

    std::unordered_set<int> visited;
    std::priority_queue<
        std::pair<double, int>,
        std::vector<std::pair<double, int>>,
        std::greater<std::pair<double, int>>
        > pq;

    for (int id : all_ids)
        distances[id] = std::numeric_limits<double>::max();

    distances[start_id] = 0.0;
    pq.push({0.0, start_id});

    while (!pq.empty())
    {
        auto [current_distance, current] = pq.top();
        pq.pop();

        if (visited.count(current))
            continue;

        visited.insert(current);

        for (int neighbor : getNeighbors(current))
        {
            const double edge_distance = getDistance(current, neighbor);
            if (edge_distance == std::numeric_limits<double>::max())
                continue;

            const double next_distance = current_distance + edge_distance;
            if (next_distance < distances[neighbor])
            {
                distances[neighbor] = next_distance;
                previous[neighbor] = current;
                pq.push({next_distance, neighbor});
            }
        }
    }

    return true;
}

bool TripPlanner::buildShortestPath(
    int start_id,
    int target_id,
    const std::unordered_map<int, double>& distances,
    const std::unordered_map<int, int>& previous,
    std::vector<int>& path
    ) const
{
    path.clear();

    const auto distance_it = distances.find(target_id);
    if (distance_it == distances.end() ||
        distance_it->second == std::numeric_limits<double>::max())
        return false;

    for (int at = target_id; ; )
    {
        path.push_back(at);
        if (at == start_id)
            break;

        const auto previous_it = previous.find(at);
        if (previous_it == previous.end())
        {
            path.clear();
            return false;
        }

        at = previous_it->second;
    }

    std::reverse(path.begin(), path.end());
    return true;
}

bool TripPlanner::buildAnnotatedRouteFromVisitOrder(
    const std::vector<int>& visit_order,
    std::vector<int>& route,
    std::vector<bool>& transit_flags
    ) const
{
    route.clear();
    transit_flags.clear();

    if (visit_order.empty())
        return false;

    route.push_back(visit_order.front());
    transit_flags.push_back(false);

    for (size_t index = 1; index < visit_order.size(); ++index)
    {
        std::unordered_map<int, double> distances;
        std::unordered_map<int, int> previous;
        if (!computeShortestPaths(route.back(), distances, previous))
            return false;

        std::vector<int> path;
        if (!buildShortestPath(route.back(), visit_order[index], distances, previous, path))
            return false;

        for (size_t path_index = 1; path_index < path.size(); ++path_index)
        {
            route.push_back(path[path_index]);
            transit_flags.push_back(path_index + 1 == path.size() ? false : true);
        }
    }

    return true;
}

TripResult TripPlanner::buildTripResultFromRoute(
    const std::vector<int>& route,
    const std::vector<bool>& transit_flags
    ) const
{
    TripResult result;
    if (route.size() != transit_flags.size())
        return result;

    double total = 0.0;

    for (size_t index = 0; index < route.size(); ++index)
    {
        result.stadiums.push_back(getStadiumById(route[index]));
        result.transit_flags.push_back(transit_flags[index]);

        if (index == 0)
            continue;

        const double segment_distance = getDistance(route[index - 1], route[index]);
        if (segment_distance == std::numeric_limits<double>::max())
            return TripResult{};

        total += segment_distance;
    }

    result.total_distance = total;
    result.total_cost = 0.0;
    return result;
}

bool TripPlanner::buildTripResultFromVisitOrder(
    const std::vector<int>& visit_order,
    double total_distance,
    TripResult& result
    ) const
{
    result = TripResult{};

    if (visit_order.empty())
        return false;

    result.transit_flags.assign(visit_order.size(), false);

    for (int stadium_id : visit_order)
    {
        Stadium stadium = getStadiumById(stadium_id);
        if (stadium.stadium_id <= 0)
        {
            result = TripResult{};
            return false;
        }

        result.stadiums.push_back(stadium);
    }

    result.total_distance = total_distance;
    result.total_cost = 0.0;
    return true;
}

std::vector<int> TripPlanner::getNeighborsSortedByDistance(int id) const
{
    std::vector<int> neighbors = getNeighbors(id);

    std::sort(neighbors.begin(), neighbors.end(), [this, id](int left, int right) {
        const double left_distance = getDistance(id, left);
        const double right_distance = getDistance(id, right);

        if (left_distance == right_distance)
            return left < right;

        return left_distance < right_distance;
    });

    return neighbors;
}

// =====================================================
// 1. Custom Ordered Trip
// =====================================================
bool TripPlanner::planCustomOrderedTrip(
    int start_stadium_id,
    const std::vector<int>& targets_in_order)
{
    if (targets_in_order.empty())
        return false;

    std::vector<int> visit_order;
    visit_order.push_back(start_stadium_id);

    for (int id : targets_in_order)
        visit_order.push_back(id);

    std::vector<int> route;
    std::vector<bool> transit_flags;
    if (!buildAnnotatedRouteFromVisitOrder(visit_order, route, transit_flags))
        return false;

    TripResult result = buildTripResultFromRoute(route, transit_flags);
    if (result.stadiums.empty())
        return false;

    _current_trip = std::make_unique<Trip>(result);
    return true;
}

// =====================================================
// 2. Greedy Unordered Trip
// =====================================================
bool TripPlanner::planCustomUnorderedEfficientTrip(
    int start_stadium_id,
    const std::vector<int>& selected_targets)
{
    if (selected_targets.empty())
        return false;

    std::unordered_set<int> remaining(
        selected_targets.begin(),
        selected_targets.end()
        );

    std::vector<int> visit_order;
    int current = start_stadium_id;
    visit_order.push_back(current);

    while (!remaining.empty())
    {
        std::unordered_map<int, double> distances;
        std::unordered_map<int, int> previous;
        if (!computeShortestPaths(current, distances, previous))
            return false;

        int nearest = -1;
        double best_dist = std::numeric_limits<double>::max();

        for (int candidate : remaining)
        {
            const auto distance_it = distances.find(candidate);
            if (distance_it != distances.end() && distance_it->second < best_dist)
            {
                best_dist = distance_it->second;
                nearest = candidate;
            }
        }

        if (nearest == -1 || best_dist == std::numeric_limits<double>::max())
            return false;

        std::vector<int> path;
        if (!buildShortestPath(current, nearest, distances, previous, path))
            return false;

        for (size_t index = 1; index < path.size(); ++index)
            remaining.erase(path[index]);

        visit_order.push_back(nearest);
        current = nearest;
    }

    std::vector<int> route;
    std::vector<bool> transit_flags;
    if (!buildAnnotatedRouteFromVisitOrder(visit_order, route, transit_flags))
        return false;

    TripResult result = buildTripResultFromRoute(route, transit_flags);
    if (result.stadiums.empty())
        return false;

    _current_trip = std::make_unique<Trip>(result);
    return true;
}

// =====================================================
// 3–7 placeholders (unchanged for now)
// =====================================================

std::vector<int> TripPlanner::getAllStadiumIds() const
{
    std::vector<int> ids;

    auto stadiums = _repo.getAllStadiums(
        StadiumRepository::StadiumSortBy::StadiumName
    );

    for (const auto& s : stadiums)
        ids.push_back(s.stadium_id);

    return ids;
}

bool TripPlanner::planShortestTripToTarget(int start_id, int target_id)
{
    if (start_id == target_id)
        return false;

    auto all_ids = getAllStadiumIds();
    std::unordered_map<int, double> dist;
    std::unordered_map<int, int> prev;
    std::unordered_set<int> visited;

    std::priority_queue<
        std::pair<double, int>,
        std::vector<std::pair<double, int>>,
        std::greater<std::pair<double, int>>
    > pq;

    for (int id : all_ids)
        dist[id] = std::numeric_limits<double>::max();

    dist[start_id] = 0.0;
    pq.push({0.0, start_id});

    while (!pq.empty())
    {
        auto [d, current] = pq.top();
        pq.pop();

        if (visited.count(current))
            continue;

        visited.insert(current);

        if (current == target_id)
            break;

        for (int neighbor : getNeighbors(current))
        {
            if (neighbor == current)
                continue;

            double w = getDistance(current, neighbor);
            if (w == std::numeric_limits<double>::max())
                continue;

            double nd = d + w;

            if (nd < dist[neighbor])
            {
                dist[neighbor] = nd;
                prev[neighbor] = current;
                pq.push({nd, neighbor});
            }
        }
    }

    if (!dist.count(target_id) ||
        dist[target_id] == std::numeric_limits<double>::max())
        return false;

    std::vector<int> path;
    for (int at = target_id; ; at = prev[at])
    {
        path.push_back(at);
        if (at == start_id) break;
    }
    std::reverse(path.begin(), path.end());

    std::vector<bool> transit_flags(path.size(), true);
    if (!transit_flags.empty())
    {
        transit_flags.front() = false;
        transit_flags.back() = false;
    }

    TripResult result = buildTripResultFromRoute(path, transit_flags);
    if (result.stadiums.empty())
        return false;

    _current_trip = std::make_unique<Trip>(result);
    return true;
}

bool TripPlanner::planVisitAllByNearestFrom(int start_id)
{
    const auto all_ids = getAllStadiumIds();
    if (all_ids.empty())
        return false;

    std::unordered_set<int> remaining(all_ids.begin(), all_ids.end());

    if (!remaining.count(start_id))
        return false;

    std::vector<int> visit_order;
    int current = start_id;
    visit_order.push_back(current);
    remaining.erase(current);

    while (!remaining.empty())
    {
        std::unordered_map<int, double> distances;
        std::unordered_map<int, int> previous;
        if (!computeShortestPaths(current, distances, previous))
            return false;

        int best = -1;
        double best_dist = std::numeric_limits<double>::max();

        for (int candidate : remaining)
        {
            const auto distance_it = distances.find(candidate);
            if (distance_it != distances.end() && distance_it->second < best_dist)
            {
                best_dist = distance_it->second;
                best = candidate;
            }
        }

        if (best == -1 || best_dist == std::numeric_limits<double>::max())
            return false;

        std::vector<int> path;
        if (!buildShortestPath(current, best, distances, previous, path))
            return false;

        for (size_t index = 1; index < path.size(); ++index)
            remaining.erase(path[index]);

        visit_order.push_back(best);
        current = best;
    }

    std::vector<int> route;
    std::vector<bool> transit_flags;
    if (!buildAnnotatedRouteFromVisitOrder(visit_order, route, transit_flags))
        return false;

    TripResult result = buildTripResultFromRoute(route, transit_flags);
    if (result.stadiums.empty())
        return false;

    _current_trip = std::make_unique<Trip>(result);
    return true;
}

bool TripPlanner::generateMSTResult()
{
    auto all_ids = getAllStadiumIds();
    if (all_ids.empty())
        return false;

    std::unordered_set<int> in_mst;

    using Edge = std::pair<double, std::pair<int,int>>;

    std::priority_queue<
        Edge,
        std::vector<Edge>,
        std::greater<Edge>
    > pq;

    int start = all_ids.front();
    in_mst.insert(start);
    std::vector<int> mst_order;
    mst_order.push_back(start);

    for (int v : getNeighbors(start))
    {
        const double distance = getDistance(start, v);
        if (distance != std::numeric_limits<double>::max())
            pq.push({distance, {start, v}});
    }

    double total = 0.0;

    while (!pq.empty() && in_mst.size() < all_ids.size())
    {
        auto [w, edge] = pq.top();
        pq.pop();

        int u = edge.first;
        int v = edge.second;

        if (in_mst.count(v))
            continue;

        if (w == std::numeric_limits<double>::max())
            return false;

        in_mst.insert(v);
        mst_order.push_back(v);
        total += w;

        for (int next : getNeighbors(v))
        {
            if (!in_mst.count(next))
            {
                const double distance = getDistance(v, next);
                if (distance != std::numeric_limits<double>::max())
                    pq.push({distance, {v, next}});
            }
        }
    }

    if (in_mst.size() != all_ids.size())
        return false;

    TripResult result;
    result.transit_flags.assign(mst_order.size(), false);

    for (int stadium_id : mst_order)
        result.stadiums.push_back(getStadiumById(stadium_id));

    result.total_distance = total;
    result.total_cost = 0.0;

    _current_trip = std::make_unique<Trip>(result);
    return true;
}

bool TripPlanner::generateDFSResultFrom(int start_id)
{
    const auto all_ids = getAllStadiumIds();
    if (std::find(all_ids.begin(), all_ids.end(), start_id) == all_ids.end())
        return false;

    std::unordered_set<int> visited;
    std::vector<int> order;
    double total_distance = 0.0;

    std::function<void(int)> dfs = [&](int current) {
        visited.insert(current);
        order.push_back(current);

        for (int neighbor : getNeighborsSortedByDistance(current))
        {
            if (!visited.count(neighbor))
            {
                const double edge_distance = getDistance(current, neighbor);
                if (edge_distance == std::numeric_limits<double>::max())
                    continue;

                total_distance += edge_distance;
                dfs(neighbor);
            }
        }
    };

    dfs(start_id);

    TripResult result;
    if (!buildTripResultFromVisitOrder(order, total_distance, result))
        return false;

    _current_trip = std::make_unique<Trip>(result);
    return true;
}

bool TripPlanner::generateBFSResultFrom(int start_id)
{
    const auto all_ids = getAllStadiumIds();
    if (std::find(all_ids.begin(), all_ids.end(), start_id) == all_ids.end())
        return false;

    std::unordered_set<int> visited;
    std::queue<int> q;
    std::vector<int> order;
    double total_distance = 0.0;

    q.push(start_id);
    visited.insert(start_id);

    while (!q.empty())
    {
        const size_t level_count = q.size();
        std::vector<std::pair<int, double>> next_level_candidates;

        for (size_t level_index = 0; level_index < level_count; ++level_index)
        {
            int current = q.front();
            q.pop();
            order.push_back(current);

            for (int neighbor : getNeighborsSortedByDistance(current))
            {
                if (visited.count(neighbor))
                    continue;

                const double edge_distance = getDistance(current, neighbor);
                if (edge_distance == std::numeric_limits<double>::max())
                    continue;

                next_level_candidates.push_back({neighbor, edge_distance});
            }
        }

        std::sort(
            next_level_candidates.begin(),
            next_level_candidates.end(),
            [](const auto& left, const auto& right) {
                if (left.second == right.second)
                    return left.first < right.first;

                return left.second < right.second;
            });

        for (const auto& [neighbor, edge_distance] : next_level_candidates)
        {
            if (visited.insert(neighbor).second)
            {
                total_distance += edge_distance;
                q.push(neighbor);
            }
        }
    }

    TripResult result;
    if (!buildTripResultFromVisitOrder(order, total_distance, result))
        return false;

    _current_trip = std::make_unique<Trip>(result);
    return true;
}


std::vector<int> TripPlanner::getNeighbors(int id) const
{
    std::vector<int> neighbors;

    QSqlDatabase db = _repo.getDatabaseManager().getDatabaseObj();
    QSqlQuery q(db);

    q.prepare(R"(
        SELECT stadium_a_id, stadium_b_id
        FROM stadium_distances
        WHERE stadium_a_id = ? OR stadium_b_id = ?
    )");

    q.addBindValue(id);
    q.addBindValue(id);

    if (q.exec())
    {
        while (q.next())
        {
            int a = q.value(0).toInt();
            int b = q.value(1).toInt();

            if (a == id)
                neighbors.push_back(b);
            else
                neighbors.push_back(a);
        }
    }

    return neighbors;
}
