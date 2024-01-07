#include "stat_reader.h"
#include <iostream>

std::pair<StringType, std::string_view> ParseRequest(std::string_view str) {
    size_t spacePos = str.find(' ');
    if (spacePos == std::string_view::npos) {
        throw std::invalid_argument("Invalid format: no space found");
    }

    std::string_view type = str.substr(0, spacePos);
    std::string_view x = str.substr(spacePos + 1);

    if (type == "Bus") {
        return {StringType::Bus, x};
    } else if (type == "Stop") {
        return {StringType::Stop, x};
    } else {
        return {StringType::Unknown, ""};
    }
}


void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output) {
    
    auto str_info = ParseRequest(request);

    if (str_info.first == StringType::Bus) {
        if (tansport_catalogue.GetRoute(str_info.second)) {
            output << "Bus " << str_info.second << ": " << tansport_catalogue.GetRoute(str_info.second)->size();
            output << " stops on route, " << tansport_catalogue.CountRouteUniqueStops(str_info.second);
            output << " unique stops, " << tansport_catalogue.GetRouteDistance(str_info.second);
            output << " route length\n";
        } 
        else {
            output << "Bus " << str_info.second << ": " << "not found\n";
        }
    } 
    else {
        output << "Stop " << str_info.second << ": ";
        if (tansport_catalogue.GetStop(str_info.second)) {
            const std::set<std::string_view>* buses = tansport_catalogue.GetStopInfo(str_info.second);
            if (buses->size() == 0) {
                output << "no buses\n";
            } 
            else {
                output << "buses ";
                for(const auto& bus : *buses) {
                    output << bus << " ";
                }
                output << "\n";
            }
        } 
        else {
            output << "not found\n";
        }
    }    

}


