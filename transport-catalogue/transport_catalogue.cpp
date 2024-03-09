#include "transport_catalogue.h"

void TransportCatalogue::AddStop(const std::string_view stop_name, Coordinates coordinates) {
    const auto stop_name_it = stops_storage_.emplace(stop_name).first;
    stops_[*stop_name_it] = Stop(*stop_name_it, coordinates);
    stops_to_buses_[*stop_name_it] = {};
}

void TransportCatalogue::AddDistanceBetweenStops(const std::string_view from_stop, const std::string_view to_stop, int dist) {
    const auto from_it = stops_storage_.emplace(from_stop).first;
    const auto to_it = stops_storage_.emplace(to_stop).first;

    distances_between_stops_[{*from_it, *to_it}] = dist;
}

void TransportCatalogue::AddRoute(std::string_view route_num, const std::vector<std::string_view> &route, bool is_rountrip) {
    const auto route_num_it = buses_storage_.emplace(route_num).first;
    
    if (is_rountrip) {
        round_trips.insert(*route_num_it);
    }

    for(const auto stop : route) {
        stops_to_buses_[stop].insert(*route_num_it);
        routes_[*route_num_it].push_back(&stops_.at(stop));
    }
}

RouteInfo TransportCatalogue::GetRouteInfo(std:: string_view route) const {
    if (GetRoute(route) != nullptr) {
        auto [road_d, geo_d] = GetRoadGeoDistance(route);
        return {route, CountRouteUniqueStops(route), static_cast<int>(GetRoute(route)->size()), road_d, road_d / geo_d};
    } 
    return {route, 0, 0, 0.0, 0.0};
}

StopInfo TransportCatalogue::GetStopInfo(std::string_view stop) const {
    if (stops_to_buses_.count(stop) == 0) {
        return {Stop(), nullptr};
    }

    return {*GetStop(stop), &stops_to_buses_.at(stop)};
}

// Get all sorted routes name
std::vector<std::string_view> TransportCatalogue::GetRoutesName() const {
    std::vector<std::string_view> all_routes;
    for (const auto& [route_name, stops] : routes_) {
        all_routes.push_back(route_name);
    }
    std::sort(all_routes.begin(), all_routes.end());
    return all_routes;
}

// Get all sorted stops name
std::vector<std::string_view> TransportCatalogue::GetStopsName() const {
    std::vector<std::string_view> all_stops;
    for (const auto& [stop, buses] : stops_to_buses_) {
        all_stops.push_back(stop);
    }
    std::sort(all_stops.begin(), all_stops.end());
    return all_stops;
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

int TransportCatalogue::GetRoadDistanceBetweenStops(const std::string_view stop1, const std::string_view stop2) const {
    auto res = distances_between_stops_.find({stop1, stop2});
    if (res != distances_between_stops_.end()) {
        return (*res).second;
    } 

    res = distances_between_stops_.find({stop2, stop1});
    if (res != distances_between_stops_.end()) {
        return (*res).second;
    } 

    return 0;
}

Distance TransportCatalogue::GetRoadGeoDistance(std::string_view route_num) const {
    if(routes_.count(route_num) == 0) {
        throw std::out_of_range("not found");
    }

    double road_dist = 0.0;
    double geo_dist = 0.0;

    const auto &route = routes_.at(route_num);

    const size_t route_size = route.size();
    for (size_t i = 1; i < route_size; ++i) {
        road_dist += GetRoadDistanceBetweenStops(route[i-1]->name_, route[i]->name_);
        geo_dist += ComputeDistance(route[i-1]->coordinates_, route[i]->coordinates_);
    }
    
    return {road_dist, geo_dist};
}

int TransportCatalogue::CountRouteUniqueStops(std::string_view route_num) const { 
    if(routes_.count(route_num) == 0) {
        throw std::out_of_range("not found");
    }

    std::unordered_set<std::string_view> unique_stops;
    const auto &route = routes_.at(route_num);
    for (const auto *const stops : route) {
        unique_stops.insert(stops->name_);
    }

    return static_cast<int>(unique_stops.size());
}