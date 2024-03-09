#pragma once

#include <string>
#include <string_view>
#include <set>
#include <unordered_map>

#include "geo.h"
#include "json.h"

using namespace geo;

// --------------Structures for stops and routes--------------
struct Stop
{
	std::string_view name_;
	Coordinates coordinates_ = {0, 0};

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
	double curvature_ = 0.0;

	RouteInfo() = default;

	RouteInfo(std::string_view route, int unique_stops, int stops_num, double route_len, double curvature) 
		: name_(route)
		, unique_stops_(unique_stops)
		, stops_num_(stops_num)
		, distance_(route_len)
		, curvature_(curvature)
	{
	}
};

struct StopInfo
{
	Stop stop_;
	const std::set<std::string_view>* buses_ = nullptr;

	StopInfo() = default;

	StopInfo(Stop stop, const std::set<std::string_view>* buses)
		: stop_(stop)
		, buses_(buses)
	{
	}
};

struct Distance
{
	double road_dist = 0.0;
	double geo_dist = 0.0;

	Distance(double road_d, double geo_d) 
		: road_dist(road_d)
		, geo_dist(geo_d)
	{
	}
};


// ------------Structures and classes for json_reader------------
enum class RequestType {
    Stop,
    Bus,
    Map
};

struct StatRequest 
{
    int id;
    RequestType type;
    std::string name;
};

class BaseRequestsDescription
{
public:
    RequestType GetType() const {
        return type_;
    }

    void SetName(std::string_view name) {
        name_ = name;
    }

    const std::string_view GetName() const {
        return name_;
    }

    virtual ~BaseRequestsDescription() = default;

protected:
    BaseRequestsDescription(RequestType type) : type_(type) { }

private:
    RequestType type_;
    std::string name_;
};

class StopRequest : public BaseRequestsDescription
{
public:
    StopRequest() : BaseRequestsDescription(RequestType::Stop) { }

    Coordinates GetCoordinates() const {
        return coordinates;
    }

    void SetCoordinates(Coordinates stop_coordinates) {
        coordinates = stop_coordinates;
    }

    const std::unordered_map<std::string, int>& GetRoadDist() const {
        return road_distances;
    }

    void SetRoadDist(const json::Dict& road_dist) {
        for (const auto& [key, val] : road_dist) {
            road_distances[key] = val.AsInt();
        }
    }

private:
    Coordinates coordinates;
    std::unordered_map<std::string, int> road_distances;
};


struct BusRequest : public BaseRequestsDescription
{
public:
    BusRequest() : BaseRequestsDescription(RequestType::Bus) { }

    const std::vector<std::string_view>& GetStops() const {
        return stops;
    }

    void SetStops(const json::Array& bus_stops) {
        for(const auto& stop : bus_stops) {
            stops.push_back(stop.AsString());
        }
    }

    bool isRoundtrip() const {
        return is_roundtrip;
    }

    void SetRoundtrip(bool roundtrip) {
        is_roundtrip = roundtrip;
    }

private:
    bool is_roundtrip;
    std::vector<std::string_view> stops;
};