#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <QString>
#include <vector>

struct Stadium
{
	int stadium_id = -1;

	QString team_name;
	QString stadium_name;
	int seating_capacity = 0;
	QString location;
	QString playing_surface;
	QString league;
	int date_opened = 0;//in year

	int distance_to_center_field_ft = 0;
	QString distance_to_center_field_raw;

	QString ballpark_typology;
	QString roof_type;
	bool is_expansion = false;
};

struct Souvenir
{
	int souvenir_id = -1;
	int owner_stadium_id = -1;

	QString name;
	double price = 0.0;
};

struct DistanceEdge
{
	int from_stadium_id = -1;
	int to_stadium_id = -1;
	double distance = 0;
};

struct DistanceNode
{
	int stadium_id = -1;
	std::vector<DistanceEdge> edges;
};

struct CartItem
{
	Souvenir item_souvenir;
	int quantity = 0;
};

struct TripResult
{
	std::vector<Stadium> stadiums;
	double total_distance = 0.0;
	double total_cost = 0.0;
};

#endif // DATA_TYPES_H
