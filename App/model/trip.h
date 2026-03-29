#ifndef TRIP_H
#define TRIP_H
#include "shopping_cart.h"
#include "../../data_types.h"

class Trip
{
public:
    Trip(TripResult result);
    ~Trip();

    bool goNextStop();
    bool goBackLastStop();

    Stadium* getCurrentStop();
    const Stadium* getCurrentStop() const;

    ShoppingCart* getShoppingCart();
    const ShoppingCart* getShoppingCart() const;

    bool hasStops() const;
    bool isAtFirstStop() const;
    bool isAtLastStop() const;
    size_t stopCount() const;
    size_t currentStopIndex() const;

private:
    size_t _current_stop_index = 0;

    TripResult _all_trip_stop;
    ShoppingCart _cart;
};

#endif // TRIP_H
