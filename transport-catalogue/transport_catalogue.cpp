#include <unordered_set>

#include "transport_catalogue.h"

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

    const Bus* TransportCatalogue::FindBus(const string& bus_name) const {
        auto find_bus = busname_to_bus_.find(bus_name);
        return find_bus == busname_to_bus_.end() ? nullptr : find_bus->second;
    }

    const Stop* TransportCatalogue::FindStop(const string& stop_name) const {
        auto find_stop = stopname_to_stops_.find(stop_name);
        return find_stop == stopname_to_stops_.end() ? nullptr : find_stop->second;
    }

    // Bus X: R stops on route, U unique stops, L route length 
    const optional<StatBus> TransportCatalogue::GetInfoBus(const string& bus_name) const {
        if (!busname_to_bus_.count(bus_name)) {
            return nullopt;
        }

        StatBus stat;
        auto bus = busname_to_bus_.at(bus_name);
        stat.stops_on_route = bus->bus.size();
        unordered_set<const Stop*> unique_stops;
        bool first = true;

        for (size_t i = 0; i < bus->bus.size(); ++i) {
            unique_stops.insert(bus->bus[i]);
            if (!first) {
                stat.route_length += ComputeDistance(bus->bus[i-1]->coordinates, bus->bus[i]->coordinates);
            }
            first = false;
        }

        stat.unique_stops = unique_stops.size();
        return stat;
    }

    // Stop X: buses bus1 bus2 ... busN 
    const set<string>* TransportCatalogue::GetInfoStop(const string& stop_name) const {
        /*
        if (!stop_to_buses_.count(stop_name)) {
            return nullopt;
        }
        return &stop_to_buses_.at(stop_name);
        */
       auto info_stop = stop_to_buses_.find(stop_name);
       return info_stop == stop_to_buses_.end() ? nullptr : &info_stop->second;
    }
}

