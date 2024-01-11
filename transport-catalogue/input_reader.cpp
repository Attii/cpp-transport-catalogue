#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

#include <iostream>

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

using StopDescription = std::pair<Coordinates, std::unordered_map<std::string_view, int>>; 

StopDescription ParseStopDescription(std::string_view line) {
    std::unordered_map<std::string_view, int> distances_to;

    auto second_comma = line.find(',',  line.find(',') + 1);

    if (second_comma != line.npos)  {
        // Получаем вектор string_view типа "DNm to stopN"
        std::vector<std::string_view> distances_to_stops = Split(line.substr(second_comma+1), ',');

        int dist;

        for (auto dist_to_stop : distances_to_stops) {
            dist = std::stoi(std::string(dist_to_stop.substr(0, dist_to_stop.find('m'))));

            auto stop_begin = dist_to_stop.find_first_not_of(' ', dist_to_stop.find('o') + 1);

            distances_to[dist_to_stop.substr(stop_begin)] = dist;
        }         
    }

     
    return {ParseCoordinates(line.substr(0, second_comma)), distances_to};
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    std::vector<const CommandDescription*> stop_commands;
    std::vector<const CommandDescription*> bus_commands;

    for (const auto& commandDes : commands_) {
        if (commandDes) {
            if (commandDes.command == "Stop") {
                stop_commands.push_back(&commandDes);
            } 

            if (commandDes.command == "Bus") {
                bus_commands.push_back(&commandDes);
            }
        }
    }
        
    for (const auto& commandDes : stop_commands) {
        auto [coordinates, distances] = ParseStopDescription(commandDes->description);

        catalogue.AddStop(commandDes->id, coordinates);
        if (!distances.empty()) {
            for (auto [stop, dist] : distances) {
                catalogue.AddDistanceBetweenStops(commandDes->id, stop, dist);
            }
        }
    }

    for (const auto& commandDes : bus_commands) {
        catalogue.AddRoute(commandDes->id, ParseRoute(commandDes->description));
    }
}