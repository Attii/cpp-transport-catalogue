#pragma once
#include <string>
#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <vector>
#include <stdexcept>
#include <set>

#include "geo.h"

struct Stop
{
	std::string_view name_;
	Coordinates coordinates_;

	Stop() = default;

	Stop(std::string_view name, Coordinates coordinates) 
		: name_(name)
		, coordinates_(coordinates)
	{
	}

	bool operator==(const Stop &other) {
		return (name_ == other.name_) && (coordinates_  == other.coordinates_);
	}
};

struct CompareStrings{
	bool operator()(const Stop& lhs, const Stop& rhs) {
		return lhs.name_ < rhs.name_;
	}
};

// class StopHasher {
// public:
// 	size_t operator()(const Stop* stop) const {
// 		return static_cast<size_t>(hasher(stop->name_));
// 	}

// private:
// 	std::hash<std::string_view> hasher;
// };

class TransportCatalogue {
public:
	void AddRoute(std::string_view route_num, const std::vector<std::string_view> route);

	void AddStop(const std::string_view stop_name, Coordinates coordinates);

	const std::deque<const Stop*>* GetRoute(std::string_view route_num) const;

	const Stop* GetStop(std::string_view stop_name) const; 

	double GetRouteDistance(std::string_view route_num) const;

	int CountRouteUniqueStops(std::string_view route_num) const;

	const std::set<std::string_view>* GetStopInfo(std::string_view stop) const;

private:
	std::unordered_set<std::string> storage_;
	std::unordered_map<std::string_view, Stop> stops_;
	std::unordered_map<std::string_view, std::deque<const Stop*>> routes_; // <route number, route in deque>
	std::unordered_map<std::string_view, std::set<std::string_view>> stops_to_buses_; 
};