#include "json_reader.h"
#include "json.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <cmath>

#include <iostream>

using namespace geo;
using namespace json;

Offset ParseOffset(const Array& arr) {
    return {arr[0].AsDouble(), arr[1].AsDouble()};
}

svg::Color ParseColor(const Node& node) {
    if (node.IsString()) {
        return node.AsString();
    }
    else {
        const auto& arr = node.AsArray();
        uint8_t r = arr[0].AsInt();
        uint8_t g = arr[1].AsInt();
        uint8_t b = arr[2].AsInt();
        if (node.AsArray().size() == 3) {
            return svg::Rgb(r, g, b);
        } 
        else {
            return svg::Rgba(r,g,b, arr[3].AsDouble());
        }
    }
}

std::vector<svg::Color> ParseColorPalette(const Array& arr) {
    std::vector<svg::Color> res;
    for(const auto& color : arr) {
        res.push_back(ParseColor(color));
    }
    return res;
}

void SetRenderSettings(RenderSettings& settings, const Dict& dict) {
    //RenderSettings settings;
    if (dict.count("width")) {
        settings.SetWidth(dict.at("width").AsDouble());
    }
    if (dict.count("height")) {
        settings.SetHeight(dict.at("height").AsDouble());
    }
    if (dict.count("padding")) {
        settings.SetPadding(dict.at("padding").AsDouble());
    }
    if (dict.count("line_width")) {
        settings.SetLineWidth(dict.at("line_width").AsDouble());
    }
    if (dict.count("stop_radius")) {
        settings.SetStopRadius(dict.at("stop_radius").AsDouble());
    }
    if (dict.count("bus_label_font_size")) {
        settings.SetBusLabelFontSize(dict.at("bus_label_font_size").AsInt());
    }
    if (dict.count("bus_label_offset")) {
        settings.SetBusLabelOffset(ParseOffset(dict.at("bus_label_offset").AsArray()));
    }
    if (dict.count("stop_label_font_size")) {
        settings.SetStopLabelFontSize(dict.at("stop_label_font_size").AsInt());
    }
    if (dict.count("stop_label_offset")) {
        settings.SetStopLabelOffset(ParseOffset(dict.at("stop_label_offset").AsArray()));
    }
    if (dict.count("underlayer_color")) {
        settings.SetUnderlayerColor(ParseColor(dict.at("underlayer_color")));
    }
    if (dict.count("underlayer_width")) {
        settings.SetUnderlayerWidth(dict.at("underlayer_width").AsDouble());
    }
    if (dict.count("color_palette")) {
        settings.SetColorPalette(ParseColorPalette(dict.at("color_palette").AsArray()));
    }
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
    std::string name;
    int id = dict.at("id").AsInt();
    if (dict.at("type").AsString() == "Map") {
        return {id, RequestType::Map, name};
    }
    RequestType type = dict.at("type").AsString() == "Bus" ? RequestType::Bus : RequestType::Stop;
    if (dict.count("name")) {
        name = dict.at("name").AsString();
    } 
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
        SetRenderSettings(render_settings_, node.AsMap().at("render_settings").AsMap());
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
        catalogue.AddRoute(command->GetName(), ParseRoute(command), command->isRoundtrip());
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
    if(stop_info.stop_.name_.empty() || stop_info.buses_ == nullptr) {
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

json::Node MakeMapNode(svg::Document&& doc, int request_id) {
    std::ostringstream svg_stream;

    doc.Render(svg_stream);

    return json::Node{ json::Dict{
        {"map", svg_stream.str()},
        {"request_id", json::Node(request_id)}
    }};
}

json::Array InputReader::ApplyStatRequests(const TransportCatalogue& catalogue, const MapRenderer& renderer) const {
    json::Array res;
    
    for(const auto& stat_request : stat_requests_) {
        if (stat_request.type == RequestType::Bus) {
            res.push_back(MakeRouteInfoNode(catalogue.GetRouteInfo(stat_request.name), stat_request.id));
        } 
        else if (stat_request.type == RequestType::Stop) {
            res.push_back(MakeStopInfoNode(catalogue.GetStopInfo(stat_request.name), stat_request.id));
        } 
        else if (stat_request.type == RequestType::Map) {
            res.push_back(MakeMapNode(renderer.RenderMap(catalogue), stat_request.id));
        }
    }

    return res;
}