#include "transport_catalogue.h"

#include<iostream>

void TransportCatalogue::AddStop(const std::string_view stop_name, Coordinates coordinates) {
    const auto stop_name_it = stops_storage_.emplace(stop_name).first;
    stops_[*stop_name_it] = Stop(*stop_name_it, coordinates);
    stops_to_buses_[*stop_name_it] = {};
}

void TransportCatalogue::AddRoute(std::string_view route_num, const std::vector<std::string_view> &route) {
    const auto route_num_it = buses_storage_.emplace(route_num).first;
    for(const auto stop : route) {
        stops_to_buses_[stop].insert(*route_num_it);
        routes_[*route_num_it].push_back(&stops_.at(stop));
    }
}

RouteInfo TransportCatalogue::GetRouteInfo(std:: string_view route) const {
    if (GetRoute(route) != nullptr) {
        return {route, CountRouteUniqueStops(route), static_cast<int>(GetRoute(route)->size()), GetRouteDistance(route)};
    } 
    return {route, 0, 0, 0.0};
}

StopInfo TransportCatalogue::GetStopInfo(std::string_view stop) const {
    if (stops_to_buses_.count(stop) == 0) {
        return {stop, nullptr};
    }

    return StopInfo(stop, &stops_to_buses_.at(stop));
}

const std::deque<const Stop*>* TransportCatalogue::GetRoute(std::string_view route_num) const {
    if(routes_.count(route_num) == 0) {
        return nullptr;
    }
    
    return &routes_.at(route_num); 
} 

const Stop* TransportCatalogue::GetStop(std::string_view stop_name) const {
    if (stops_.count(stop_name) == 0) {
        return nullptr;
    }

    return &(stops_.at(stop_name));
}

double TransportCatalogue::GetRouteDistance(std::string_view route_num) const {
    if(routes_.count(route_num) == 0) {
        throw std::out_of_range("not found");
    }
    double dist = 0.0;

    auto &route = routes_.at(route_num);
    for (size_t i = 1; i < route.size(); ++i) {
        dist += ComputeDistance(route[i-1]->coordinates_, route[i]->coordinates_);
    }
    
    return dist;
}

int TransportCatalogue::CountRouteUniqueStops(std::string_view route_num) const { 
    if(routes_.count(route_num) == 0) {
        throw std::out_of_range("not found");
    }

    std::unordered_set<std::string_view> unique_stops;
    auto &route = routes_.at(route_num);
    for (const auto stops : route) {
        unique_stops.insert(stops->name_);
    }

    return unique_stops.size();
}