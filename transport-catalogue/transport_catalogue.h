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
};

struct RouteInfo 
{
	const std::string_view name_;
	int unique_stops_ = 0;
	int stops_num_ = 0;
	double distance_ = 0.0;

	RouteInfo() = default;

	RouteInfo(std::string_view route, int unique_stops, int stops_num, double route_len) 
		: name_(route)
		, unique_stops_(unique_stops)
		, stops_num_(stops_num)
		, distance_(route_len)
	{
	}
};

struct StopInfo
{
	const std::string_view name_;
	const std::set<std::string_view>* buses_ = nullptr;

	StopInfo() = default;

	StopInfo(std::string_view name, const std::set<std::string_view>* buses)
		: name_(name)
		, buses_(buses)
	{
	}
};


// struct CompareStrings{
// 	bool operator()(const Stop& lhs, const Stop& rhs) {
// 		return lhs.name_ < rhs.name_;
// 	}
// };

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
	void AddRoute(std::string_view route_num, const std::vector<std::string_view> &route);
	void AddStop(const std::string_view stop_name, Coordinates coordinates);

	RouteInfo GetRouteInfo(std:: string_view route) const;
	StopInfo GetStopInfo(std::string_view stop) const;

private:
	const std::deque<const Stop*>* GetRoute(std::string_view route_num) const;

	const Stop* GetStop(std::string_view stop_name) const; 

	double GetRouteDistance(std::string_view route_num) const;

	int CountRouteUniqueStops(std::string_view route_num) const;

	std::unordered_set<std::string> stops_storage_;
	std::unordered_set<std::string> buses_storage_;

	std::unordered_map<std::string_view, Stop> stops_;
	std::unordered_map<std::string_view, std::deque<const Stop*>> routes_; // <bus , stops in deque>
	std::unordered_map<std::string_view, std::set<std::string_view>> stops_to_buses_; 
};