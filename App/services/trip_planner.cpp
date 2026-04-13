#include "trip_planner.h"

TripPlanner::TripPlanner(){}

Trip* TripPlanner::getCurrentTrip()
{
	return _current_trip.get();
}
