#include "stat_reader.h"
#include <iostream>

std::pair<RequestType, std::string_view> ParseRequest(std::string_view str) {
    size_t spacePos = str.find(' ');
    if (spacePos == std::string_view::npos) {
        throw std::invalid_argument("Invalid format: no space found");
    }

    std::string_view type = str.substr(0, spacePos);
    std::string_view id = str.substr(spacePos + 1);

    if (type == "Bus") {
        return {RequestType::Bus, id};
    } else if (type == "Stop") {
        return {RequestType::Stop, id};
    } else {
        return {RequestType::Unknown, ""};
    }
}

void PrintRouteInfo(const RouteInfo& route_info, std::ostream& output) {
    if (route_info.stops_num_ != 0) {
        output << "Bus " << route_info.name_ << ": " << route_info.stops_num_;
        output << " stops on route, " << route_info.unique_stops_;
        output << " unique stops, " << route_info.distance_;
        output << " route length, " << route_info.curvature_;
        output << " curvature\n";
    } 
    else {
        output << "Bus " << route_info.name_ << ": " << "not found\n";
    }
}

void PrintStopInfo(const StopInfo& stop_info, std::ostream& output) {
    output << "Stop " << stop_info.name_ << ": ";
        if (stop_info.buses_ != nullptr) {
            if (stop_info.buses_->size() == 0) {
                output << "no buses\n";
            } 
            else {
                output << "buses ";
                for(const auto& bus : *stop_info.buses_) {
                    output << bus << " ";
                }
                output << "\n";
            }
        } 
        else {
            output << "not found\n";
        }
}


void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output) {
    
    auto str_info = ParseRequest(request);

    if (str_info.first == RequestType::Bus) {
        PrintRouteInfo(tansport_catalogue.GetRouteInfo(str_info.second), output);
    } 
    else {
        PrintStopInfo(tansport_catalogue.GetStopInfo(str_info.second), output);
    }    

}


