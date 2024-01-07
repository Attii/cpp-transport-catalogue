#pragma once

#include <iosfwd>
#include <string_view>
#include <stdexcept>
#include <set>

#include "transport_catalogue.h"

enum class StringType {
    Bus,
    Stop,
    Unknown
};

std::pair<StringType, std::string_view> ParseRequest(std::string_view str);

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output);