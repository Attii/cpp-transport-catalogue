#include "json_reader.h"
#include "json.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <cmath>

#include <iostream>

using namespace geo;
using namespace json;

void ParseRenderSettings(const Dict& dict) {
    
}

// /**
//  * Парсит маршрут.
//  * Для кольцевого маршрута [A,B,C] возвращает массив названий остановок [A,B,C,A]
//  * Для некольцевого маршрута [A,B,C,D] возвращает массив названий остановок [A,B,C,D,C,B,A]
//  */
std::vector<std::string_view> ParseRoute(const BusRequest* bus_request) {
    const auto& stops = bus_request->GetStops();

    std::vector<std::string_view> result(stops.begin(), stops.end());

    if (!bus_request->isRoundtrip()) { 
        for (int i = static_cast<int>(stops.size()) - 2; i >= 0; --i)
        {
            result.push_back(stops[i]);
        }
    }

    return result;
}

std::unique_ptr<BaseRequestsDescription> ParseStop(const Dict& stop_info) {
    auto stop_request = std::make_unique<StopRequest>();
    Coordinates coordinates = {0,0};
    for(const auto& [key, val] : stop_info) {
        if (key == "name") {
            stop_request.get()->SetName(val.AsString());
            continue;
        }

        if (key == "latitude") {
            coordinates.lat = val.AsDouble();
            continue;
        }

        if (key == "longitude") {
            coordinates.lng = val.AsDouble();
            continue;
        }

        if (key == "road_distances") {
            stop_request.get()->SetRoadDist(val.AsMap());
            continue;
        }
    }
    stop_request.get()->SetCoordinates(coordinates);
    return stop_request;
}

std::unique_ptr<BaseRequestsDescription> ParseBus(const Dict& bus_info) {
    auto bus_request = std::make_unique<BusRequest>();
    for(const auto& [key, val] : bus_info) {
        if (key == "name") {
            bus_request.get()->SetName(val.AsString());
            continue;
        }

        if (key == "stops") {
            bus_request.get()->SetStops(val.AsArray());
            continue;
        }

        if (key == "is_roundtrip") {
            bus_request.get()->SetRoundtrip(val.AsBool());
            continue;
        }
    }
    return bus_request;
}

std::unique_ptr<BaseRequestsDescription> ParseBaseRequest(const Dict& dict) {
    if (dict.at("type").AsString() == "Bus") {
        return ParseBus(dict);
    }
    else {
        return ParseStop(dict);
    }
}

StatRequest ParseStatRequest(const Dict& dict) {
    int id = dict.at("id").AsInt();
    RequestType type = dict.at("type").AsString() == "Bus" ? RequestType::Bus : RequestType::Stop;
    std::string name = dict.at("name").AsString();
    return {id, type, name};
}

void InputReader::ParseJSON(const Node& node) {
    if (node.AsMap().count("base_requests")) {
        for (const Node& request : node.AsMap().at("base_requests").AsArray()) {
            base_requests_.emplace_back(ParseBaseRequest(request.AsMap()));
        }
    }
    
    if (node.AsMap().count("stat_requests")) {
        for(const Node& request : node.AsMap().at("stat_requests").AsArray()) {
            stat_requests_.push_back(ParseStatRequest(request.AsMap()));
        }
    }

    if (node.AsMap().count("render_settings")) {

    }
}

void InputReader::ApplyBaseRequests([[maybe_unused]] TransportCatalogue& catalogue) const {
    std::vector<const StopRequest*> stop_commands;
    std::vector<const BusRequest*> bus_commands;
    
    for(const auto& base_request : base_requests_) {
        if (base_request.get()->GetType() == RequestType::Stop) {
            if (auto stop_request = dynamic_cast<const StopRequest*>(base_request.get())) 
                stop_commands.push_back(stop_request);
        }

        if (base_request.get()->GetType() == RequestType::Bus) {
            if (auto bus_request = dynamic_cast<const BusRequest*>(base_request.get())) {
                bus_commands.push_back(bus_request);
            }
        }
    }
        
    for(const auto& commmand : stop_commands) {
        std::string_view stop_to_add = commmand->GetName(); 
        catalogue.AddStop(stop_to_add, commmand->GetCoordinates());
        if(!commmand->GetRoadDist().empty()) {
            for (const auto& [near_stop, dist_to_stop] :commmand->GetRoadDist()) {
                catalogue.AddDistanceBetweenStops(stop_to_add, near_stop, dist_to_stop);
            }
        }

    }

    for (const auto& command : bus_commands) {
        catalogue.AddRoute(command->GetName(), ParseRoute(command));
    }
}

json::Node MakeRouteInfoNode(const RouteInfo& route_info, int request_id) {
    if(route_info.stops_num_ == 0) {
        return json::Node{ json::Dict{
            {"request_id", json::Node(request_id)},
            {"error_message", json::Node(std::string("not found"))}}
        };
    }
    
    return json::Node{ json::Dict{
        {"request_id", json::Node(request_id)},
        {"unique_stop_count", json::Node(route_info.unique_stops_)},
        {"stop_count", json::Node(route_info.stops_num_)},
        {"route_length", json::Node(route_info.distance_)},
        {"curvature", json::Node(route_info.curvature_)}}
    };
}

json::Node MakeStopInfoNode(const StopInfo& stop_info, int request_id) {    
    if(stop_info.buses_ == nullptr) {
        return json::Node{ json::Dict{
            {"request_id", json::Node(request_id)},
            {"error_message", json::Node(std::string("not found"))}}
        };    
    }
    
    json::Array buses_array;
    if (stop_info.buses_) {
        for (const auto& bus : *stop_info.buses_) {
            buses_array.push_back(json::Node(std::string(bus)));
        }
    }

    return json::Node{ json::Dict{
        {"request_id", json::Node(request_id)},
        {"buses", json::Node(std::move(buses_array))}}
    };
}


json::Array InputReader::ApplyStatRequests(TransportCatalogue& catalogue) const {
    json::Array res;
    
    for(const auto& stat_request : stat_requests_) {
        if (stat_request.type == RequestType::Bus) {
            res.push_back(MakeRouteInfoNode(catalogue.GetRouteInfo(stat_request.name), stat_request.id));
        } 
        else if (stat_request.type == RequestType::Stop) {
            res.push_back(MakeStopInfoNode(catalogue.GetStopInfo(stat_request.name), stat_request.id));
        }
    }

    return res;
}