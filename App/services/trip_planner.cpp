#include "trip_planner.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <limits>
#include <algorithm>
#include <queue>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <QSqlError>

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
    {
        qDebug() << "DB NOT OPEN";
        return std::numeric_limits<double>::max();
    }

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
    {
        qDebug() << "QUERY FAILED:" << q.lastError();
        return std::numeric_limits<double>::max();
    }

    if (q.next())
    {
        double d = q.value(0).toDouble();
        qDebug() << "DIST:" << a << "->" << b << "=" << d;
        return d;
    }

    qDebug() << "NO RESULT FOR:" << a << b;
    return std::numeric_limits<double>::max();
}

// =====================================================
// Helper: load Stadium by ID
// =====================================================
Stadium TripPlanner::getStadiumById(int id)
{
    auto s = _repo.getStadiumByID(id);
    if (s.has_value())
        return *s;

    return Stadium{};
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

    TripResult result;

    std::vector<int> route;
    route.push_back(start_stadium_id);

    for (int id : targets_in_order)
        route.push_back(id);

    double total = 0.0;

    for (size_t i = 0; i < route.size(); i++)
    {
        result.stadiums.push_back(getStadiumById(route[i]));

        if (i > 0)
        {
            total += getDistance(route[i - 1], route[i]);
        }
    }

    result.total_distance = total;
    result.total_cost = 0.0;

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

    std::vector<int> route;
    int current = start_stadium_id;
    route.push_back(current);

    while (!remaining.empty())
    {
        int nearest = -1;
        double best_dist = std::numeric_limits<double>::max();

        for (int candidate : remaining)
        {
            double d = getDistance(current, candidate);
            if (d < best_dist)
            {
                best_dist = d;
                nearest = candidate;
            }
        }

        if (nearest == -1)
            return false;

        route.push_back(nearest);
        current = nearest;
        remaining.erase(nearest);
    }

    TripResult result;
    double total = 0.0;

    for (size_t i = 0; i < route.size(); i++)
    {
        result.stadiums.push_back(getStadiumById(route[i]));

        if (i > 0)
            total += getDistance(route[i - 1], route[i]);
    }

    result.total_distance = total;
    result.total_cost = 0.0;

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

    TripResult result;
    double total = 0.0;

    for (size_t i = 0; i < path.size(); i++)
    {
        result.stadiums.push_back(getStadiumById(path[i]));
        if (i > 0)
            total += getDistance(path[i - 1], path[i]);
    }

    result.total_distance = total;
    result.total_cost = 0.0;

    _current_trip = std::make_unique<Trip>(result);
    return true;
}

bool TripPlanner::planVisitAllByNearestFrom(int start_id)
{
    auto remaining = std::unordered_set<int>(
        getAllStadiumIds().begin(),
        getAllStadiumIds().end()
    );

    if (!remaining.count(start_id))
        return false;

    std::vector<int> route;
    int current = start_id;
    route.push_back(current);
    remaining.erase(current);

    while (!remaining.empty())
    {
        int best = -1;
        double best_dist = std::numeric_limits<double>::max();

        for (int candidate : remaining)
        {
            double d = getDistance(current, candidate);
            if (d < best_dist)
            {
                best_dist = d;
                best = candidate;
            }
        }

        if (best == -1)
        {
            qDebug() << "WARNING: No reachable next stadium from" << current;
            break; // instead of failing
        }

        route.push_back(best);
        remaining.erase(best);
        current = best;
    }

    TripResult result;
    double total = 0.0;

    for (size_t i = 0; i < route.size(); i++)
    {
        result.stadiums.push_back(getStadiumById(route[i]));
        if (i > 0)
            total += getDistance(route[i - 1], route[i]);
    }

    result.total_distance = total;
    result.total_cost = 0.0;

    _current_trip = std::make_unique<Trip>(result);
    return true;
}

bool TripPlanner::generateMSTResult()
{
    auto all_ids = getAllStadiumIds();;
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

    for (int v : all_ids)
    {
        if (v != start)
            pq.push({getDistance(start, v), {start, v}});
    }

    std::vector<std::pair<int,int>> mst_edges;
    double total = 0.0;

    while (!pq.empty() && in_mst.size() < all_ids.size())
    {
        auto [w, edge] = pq.top();
        pq.pop();

        int u = edge.first;
        int v = edge.second;

        if (in_mst.count(v))
            continue;

        in_mst.insert(v);
        mst_edges.push_back({u, v});
        total += w;

        for (int next : all_ids)
        {
            if (!in_mst.count(next))
                pq.push({getDistance(v, next), {v, next}});
        }
    }

    TripResult result;

    for (auto [u, v] : mst_edges)
    {
        result.stadiums.push_back(getStadiumById(u));
        result.stadiums.push_back(getStadiumById(v));
    }

    result.total_distance = total;
    result.total_cost = 0.0;

    _current_trip = std::make_unique<Trip>(result);
    return true;
}

bool TripPlanner::generateDFSResultFrom(int start_id)
{
    std::unordered_set<int> visited;
    std::vector<int> order;
    std::stack<int> st;

    st.push(start_id);

    while (!st.empty())
    {
        int current = st.top();
        st.pop();

        if (visited.count(current))
            continue;

        visited.insert(current);
        order.push_back(current);

        for (int neighbor : getNeighbors(current))
        {
            if (!visited.count(neighbor))
                st.push(neighbor);
        }
    }

    TripResult result;
    double total = 0.0;

    for (size_t i = 0; i < order.size(); i++)
    {
        result.stadiums.push_back(getStadiumById(order[i]));

        if (i > 0)
        {
            double d = getDistance(order[i - 1], order[i]);

            if (d != std::numeric_limits<double>::max())
                total += d;
        }
    }

    result.total_distance = total;
    result.total_cost = 0.0;

    _current_trip = std::make_unique<Trip>(result);
    return true;
}

bool TripPlanner::generateBFSResultFrom(int start_id)
{
    std::unordered_set<int> visited;
    std::queue<int> q;
    std::vector<int> order;

    q.push(start_id);
    visited.insert(start_id);

    while (!q.empty())
    {
        int current = q.front();
        q.pop();

        order.push_back(current);

        for (int neighbor : getNeighbors(current))
        {
            if (!visited.count(neighbor))
            {
                visited.insert(neighbor);
                q.push(neighbor);
            }
        }
    }

    TripResult result;
    double total = 0.0;

    for (size_t i = 0; i < order.size(); i++)
    {
        result.stadiums.push_back(getStadiumById(order[i]));

        if (i > 0)
        {
            double d = getDistance(order[i - 1], order[i]);

            if (d != std::numeric_limits<double>::max())
                total += d;
        }
    }

    result.total_distance = total;
    result.total_cost = 0.0;

    _current_trip = std::make_unique<Trip>(result);
    return true;
}


std::vector<int> TripPlanner::getNeighbors(int id)
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