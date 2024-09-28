#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>
#include <cmath>

using namespace std;

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
geo::Coordinates ParseCoordinates(std::string_view str) {
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

std::vector<std::pair<std::string, double>> ParseDistance(std::string str) {
    std::vector<std::pair<std::string, double>> distance;

    // Удаляем начало строки до первого ":"
    size_t pos = str.find(':');
    if (pos != std::string::npos) {
        str = str.substr(pos + 1); 
    }
    // Удаляем координаты
    pos = str.find(',');
    if (pos != std::string::npos) {
        str = str.substr(pos + 1);
    }
    pos = str.find(',');
    if (pos != std::string::npos) {
        str = str.substr(pos + 1);
    }

    str = Trim(str).data();

    // Разделяем строку на части по "to"
    size_t toPos = str.find("m to ");

    while (toPos != std::string::npos) {
        std::string dist = str.substr(0, toPos);
        str = str.substr(toPos + 5); // Удаляем "to "

        // Извлекаем название местоположения
        size_t commaPos = str.find(',');
        string stop;
        if (commaPos != std::string::npos) {
            stop = str.substr(0, commaPos);
            str = str.substr(commaPos + 1);
            str = Trim(str).data();
        } else {
            stop = str;
            str = ""s;
        }

        // Добавляем пару в вектор
        distance.emplace_back(stop, stoi(dist));

        toPos = str.find("m to ");
    }

    return distance;
}

void InputReader::ApplyCommands(TransportCatalogue::TransportCatalogue& catalogue) const {
    // Stop X: latitude, longitude, D1m to stop1, D2m to stop2, ...
    for (const CommandDescription& cd : commands_) {
        if (cd.command == "Stop"s) {
            TransportCatalogue::Stop stop;
            stop.name = cd.id;
            geo::Coordinates coordinates = ParseCoordinates(cd.description);
            stop.coordinates = move(coordinates);

            catalogue.AddStop(stop);
        }
    }

    for (const CommandDescription& cd : commands_) {

        if (cd.command == "Stop"s) {
            std::vector<std::pair<std::string, double>> dist = ParseDistance(cd.description);
            for (size_t i = 0; i < dist.size(); ++i) {
                catalogue.SetDistanceBetweenStops(cd.id, dist[i].first, dist[i].second);
            }
        }

        if (cd.command == "Bus"s) {
            TransportCatalogue::Bus bus;
            bus.name = cd.id;
            // vector<string_view> route = move(ParseRoute(cd.description));
            vector<string_view> route = ParseRoute(cd.description);

            vector<const TransportCatalogue::Stop*> stops;
            for (const string_view s : route) {
                stops.push_back(catalogue.FindStop(string(s))); 
            } 
            bus.bus = move(stops);
            catalogue.AddBus(bus);
        }
    }
}

void ParseBaseRequest(TransportCatalogue::TransportCatalogue& catalogue, std::istream& input) {
    int base_request_count;
    input >> base_request_count >> std::ws;

    InputReader reader;
    for (int i = 0; i < base_request_count; ++i) {
        string line;
        getline(input, line);
        reader.ParseLine(line);
    }
    reader.ApplyCommands(catalogue);
}
