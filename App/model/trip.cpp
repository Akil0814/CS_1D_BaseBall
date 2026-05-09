#include "trip.h"

Trip::Trip(TripResult result)
    : _all_trip_stop(result)
{}

Trip::~Trip() = default;

bool Trip::goNextStop()
{
    if (!hasStops())
        return false;

    if (isAtLastStop())
        return false;

    ++_current_stop_index;
    return true;
}

bool Trip::goBackLastStop()
{
    if (!hasStops())
        return false;

    if (isAtFirstStop())
        return false;

    --_current_stop_index;
    return true;
}

Stadium* Trip::getCurrentStop()
{
    if (!hasStops())
        return nullptr;

    if (_current_stop_index >= _all_trip_stop.stadiums.size())
        return nullptr;

    return &_all_trip_stop.stadiums[_current_stop_index];
}

const Stadium* Trip::getCurrentStop() const
{
    if (!hasStops())
        return nullptr;

    if (_current_stop_index >= _all_trip_stop.stadiums.size())
        return nullptr;

    return &_all_trip_stop.stadiums[_current_stop_index];
}

ShoppingCart* Trip::getShoppingCart()
{
    return &_cart;
}

const ShoppingCart* Trip::getShoppingCart() const
{
    return &_cart;
}

bool Trip::addSouvenirToCart(const Souvenir& souvenir, int quantity)
{
    if (!_cart.addItem(souvenir, quantity))
        return false;

    _all_trip_stop.total_cost = _cart.totalCost();
    return true;
}

bool Trip::removeCartItemAt(size_t index)
{
    if (!_cart.removeItemAt(index))
        return false;

    _all_trip_stop.total_cost = _cart.totalCost();
    return true;
}

void Trip::clearShoppingCart()
{
    _cart.clear();
    _all_trip_stop.total_cost = 0.0;
}

double Trip::totalCost() const
{
    return _cart.totalCost();
}

bool Trip::hasStops() const
{
    return !_all_trip_stop.stadiums.empty();
}

bool Trip::isAtFirstStop() const
{
    if (!hasStops())
        return false;

    return _current_stop_index == 0;
}

bool Trip::isAtLastStop() const
{
    if (!hasStops())
        return false;

    return _current_stop_index == _all_trip_stop.stadiums.size() - 1;
}

size_t Trip::stopCount() const
{
    return _all_trip_stop.stadiums.size();
}

size_t Trip::currentStopIndex() const
{
    return _current_stop_index;
}

//for trip_planner_cli (testing
const TripResult& Trip::getResult() const {
    return _all_trip_stop;
}
