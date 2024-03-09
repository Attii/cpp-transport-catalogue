#pragma once

#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <vector>
#include <stdexcept>
#include <set>
#include <algorithm>

#include "domain.h"
#include "geo.h"

using namespace geo;

class PairStopsHasher {
public:
	size_t operator()(const std::pair<std::string_view, std::string_view> var) const {
		return static_cast<size_t>(hasher_(var.first) + 17*hasher_(var.second)); 
	}

private:
	std::hash<std::string_view> hasher_;
};

class TransportCatalogue {
public:
	void AddRoute(std::string_view route_num, const std::vector<std::string_view> &route, bool is_rountrip);
	void AddStop(const std::string_view stop_name, Coordinates coordinates);
	void AddDistanceBetweenStops(const std::string_view from_stop, const std::string_view to_stop, int dist);

	bool IsRoundTrip(const std::string_view route) const {return round_trips.count(route);}

	[[nodiscard]] RouteInfo GetRouteInfo(std:: string_view route) const;
	[[nodiscard]] StopInfo GetStopInfo(std::string_view stop) const;

	const std::unordered_set<std::string>& GetStops() const {return stops_storage_;}

	const std::unordered_map<std::string_view, std::deque<const Stop*>>& GetRoutes() const {return routes_;}

	std::vector<std::string_view> GetRoutesName() const;
	std::vector<std::string_view> GetStopsName() const;


private:
	[[nodiscard]] const std::deque<const Stop*>* GetRoute(std::string_view route_num) const;

	[[nodiscard]] const Stop* GetStop(std::string_view stop_name) const; 

	[[nodiscard]] Distance GetRoadGeoDistance(std::string_view route_num) const;

	[[nodiscard]] int CountRouteUniqueStops(std::string_view route_num) const;

	[[nodiscard]] int GetRoadDistanceBetweenStops(const std::string_view stop1, const std::string_view stop2) const;

	std::unordered_set<std::string> stops_storage_;
	std::unordered_set<std::string> buses_storage_;

	std::unordered_set<std::string_view> round_trips; // к сожалению, это костыль

	std::unordered_map<std::string_view, Stop> stops_;
	std::unordered_map<std::string_view, std::deque<const Stop*>> routes_; // <bus , stops in deque>
	std::unordered_map<std::string_view, std::set<std::string_view>> stops_to_buses_; 
	std::unordered_map<std::pair<std::string_view, std::string_view>, int,  PairStopsHasher> distances_between_stops_;
};