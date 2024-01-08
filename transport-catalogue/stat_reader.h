#pragma once

#include <iosfwd>
#include <string_view>
#include <stdexcept>
#include <set>

#include "transport_catalogue.h"

enum class RequestType {
    Bus,
    Stop,
    Unknown
};

std::pair<RequestType, std::string_view> ParseRequest(std::string_view str);

void PrintRouteInfo(const RouteInfo& route_info, std::ostream& output);
void PrintStopInfo(const StopInfo& stop_info, std::ostream& output);

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output);