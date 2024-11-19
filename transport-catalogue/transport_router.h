#pragma once

#include "transport_catalogue.h"
#include "router.h"

#include <unordered_map>
#include <string_view>
#include <memory>
#include <optional>

static constexpr inline double COEFFICIENT = 0.06; // 60 минут/1000 метров

struct Settings {
    int wait_time = 0;
    double velocity = 0;
};

struct Passage {
    std::string_view start;
    std::string_view bus;
    int span_count = 0;
    int wait_time = 0;
    double total_time = 0;

    bool operator<(const Passage& other) const {
        return total_time < other.total_time;
    }

    bool operator>(const Passage& other) const {
        return total_time > other.total_time;
    }

    Passage operator+(const Passage& other) const {
        return {std::string_view{}, std::string_view{}, 0, 0, total_time + other.total_time};
    }
};

using Catalogue = TransportCatalogue::TransportCatalogue;
using Stop = TransportCatalogue::Stop;
using Bus = TransportCatalogue::Bus;

struct RouteInfo {
    double total_time = 0;
    std::vector<const Passage*> passages;
};

class Router {
public:
    Router(const Catalogue& catalogue, Settings settings);

    std::optional<RouteInfo> GetRouteInfo(std::string_view from, std::string_view to) const;

private:
    const Catalogue& catalogue_;
    Settings settings_;

    std::unordered_map<std::string_view, size_t> stops_id_;
    std::unique_ptr<graph::DirectedWeightedGraph<Passage>> graph_;
    std::unique_ptr<graph::Router<Passage>> router_;



    void CreateStopsId(std::vector<const Stop*>&& path);

    size_t GetStopId(std::string_view name) const;

    void CreateGraph();

    void AddEdges(const Bus& bus);

    ranges::Range<typename std::vector<const Stop*>::const_iterator> GetForward(const Bus& bus);

    ranges::Range<typename std::vector<const Stop*>::const_iterator> GetBackward(const Bus& bus);

    ranges::Range<typename std::vector<const Stop*>::const_iterator> GetRound(const Bus& bus);

    void AddEdges(ranges::Range<typename std::vector<const Stop*>::const_iterator> range, std::string_view bus);

};