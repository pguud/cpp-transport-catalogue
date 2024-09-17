#include <unordered_set>

#include "transport_catalogue.h"

using namespace std;

namespace TransportCatalogue {
    void TransportCatalogue::AddBus(const Bus& bus) {
        buses_.push_back(bus);
        busname_to_bus_[bus.name] = &buses_.back();
        
        for (size_t i = 0; i < bus.bus.size(); ++i) {
            stop_to_buses_[bus.bus[i]->name].insert(bus.name);
        }
    }

    void TransportCatalogue::AddStop(const Stop& stop) {
        stops_.push_back(stop);
        stopname_to_stops_[stop.name] = &stops_.back();

        stop_to_buses_[stop.name];
    }

    const Bus* TransportCatalogue::FindBus(const string_view& bus_name) const {
        auto find_bus = busname_to_bus_.find(bus_name.data());
        return find_bus == busname_to_bus_.end() ? nullptr : find_bus->second;
    }

    const Stop* TransportCatalogue::FindStop(const string_view& stop_name) const {
        auto find_stop = stopname_to_stops_.find(stop_name.data());
        return find_stop == stopname_to_stops_.end() ? nullptr : find_stop->second;
    }

    // Bus X: R stops on route, U unique stops, L route length 
    // ИЗМЕНЕНО на : 
    // Bus X: R stops on route, U unique stops, L route length, C curvature
    // L - дорожное расстояние
    // С — извилистость, то есть отношение фактической длины маршрута к географическому расстоянию.
    const optional<StatBus> TransportCatalogue::GetInfoBus(const string_view& bus_name) const {
        if (!busname_to_bus_.count(bus_name.data())) {
            return nullopt;
        }

        StatBus stat;
        auto bus = busname_to_bus_.at(bus_name.data());
        stat.stops_on_route = bus->bus.size();
        unordered_set<const Stop*> unique_stops;
        bool first = true;

        double geographic_distance = 0;
        for (size_t i = 0; i < bus->bus.size(); ++i) {
            unique_stops.insert(bus->bus[i]);
            if (!first) {
                geographic_distance += ComputeDistance(bus->bus[i-1]->coordinates, bus->bus[i]->coordinates);
                stat.route_length += GetDistanceBetweenStops(bus->bus[i-1], bus->bus[i]);
            }
            first = false;
        }

        stat.unique_stops = unique_stops.size();

        stat.curvature = stat.route_length / geographic_distance;

        return stat;
    }

    // Stop X: buses bus1 bus2 ... busN 
    const set<string>* TransportCatalogue::GetInfoStop(const string_view& stop_name) const {
       auto info_stop = stop_to_buses_.find(stop_name.data());
       return info_stop == stop_to_buses_.end() ? nullptr : &info_stop->second;
    }

    // задание дистанции между остановками.
    void TransportCatalogue::SetDistanceBetweenStops(const std::string_view& first_stop_name, 
                                const std::string_view& second_stop_name, double dist) {
        const Stop* first_stop = FindStop(first_stop_name);
        const Stop* second_stop = FindStop(second_stop_name);

        distance_between_stops_[{first_stop, second_stop}] = dist;
    }

    // получение дистанции между остановками.
    double TransportCatalogue::GetDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const {

        if (distance_between_stops_.find({stop1, stop2}) != distance_between_stops_.end()) {
            return distance_between_stops_.at({stop1, stop2});
        }

        return distance_between_stops_.at({stop2, stop1});
    }
}
