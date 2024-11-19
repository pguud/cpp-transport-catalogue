#include "transport_router.h"

#include <iostream>

    Router::Router(const Catalogue& catalogue, Settings settings) 
        : catalogue_(catalogue), settings_(settings) {
            
            CreateStopsId(catalogue_.GetActualStops());
            
            CreateGraph();
            router_ = std::make_unique<graph::Router<Passage>>(*graph_);
    }

    std::optional<RouteInfo> Router::GetRouteInfo(std::string_view from, std::string_view to) const {
        if (catalogue_.IsSingleStop(from) || catalogue_.IsSingleStop(to)) {
            return std::nullopt;
        }

        std::optional<graph::Router<Passage>::RouteInfo> route = router_->BuildRoute(GetStopId(std::string(from)), GetStopId(std::string(to)));

        if (!route) {
            return std::nullopt;
        }

        RouteInfo result;

        result.total_time = route->weight.total_time;

        for (const auto& edge: route->edges) {
            result.passages.emplace_back(&graph_->GetEdge(edge).weight);
        }

        return result;
    }
    
    void Router::CreateStopsId(std::vector<const Stop*>&& path) {
        size_t id = 0;
        for (const Stop* stop: path) {
            stops_id_[stop->name] = id++;

        }
    }

    size_t Router::GetStopId(std::string_view name) const {
        auto stop = stops_id_.find(name);
        assert(stop != stops_id_.end());

        return stop->second;
    }

    void Router::CreateGraph() {
        graph_ = std::make_unique<graph::DirectedWeightedGraph<Passage>>(stops_id_.size());

        for (const Bus* bus: catalogue_.GetActualBuses()) {
            AddEdges(*bus);
        }
    }

    void Router::AddEdges(const Bus& bus) {
        auto& [name, _, is_round] = bus;

        if (is_round) {
            AddEdges(GetRound(bus), name);
        } else {
            AddEdges(GetForward(bus), name);
            AddEdges(GetBackward(bus), name);
        }
    }

    ranges::Range<typename std::vector<const Stop*>::const_iterator> Router::GetForward(const Bus& bus) {
        auto middle = next(bus.bus.begin(), static_cast<std::iterator_traits<std::vector<const Stop*>::iterator>::difference_type>(bus.bus.size() / 2));
        return {bus.bus.begin(), next(middle)};
    }

    ranges::Range<typename std::vector<const Stop*>::const_iterator> Router::GetBackward(const Bus& bus) {
        auto middle = next(bus.bus.begin(), static_cast<std::iterator_traits<std::vector<const Stop*>::iterator>::difference_type>(bus.bus.size() / 2));
        return {middle, bus.bus.end()};
    }

    ranges::Range<typename std::vector<const Stop*>::const_iterator> Router::GetRound(const Bus& bus) {
        return {bus.bus.begin(), bus.bus.end()};
    }

    void Router::AddEdges(ranges::Range<typename std::vector<const Stop*>::const_iterator> range, std::string_view bus) {
        auto begin = range.begin();
        auto end = range.end();

        auto [wait_time, velocity] = settings_;

        while (begin != end) {
            std::unordered_set<size_t> passed;
            int span_count = 0;
            double total_distance = 0;

            std::string start = (*begin)->name;
            size_t start_id = GetStopId(start);

            auto stop_a = begin;
            auto stop_b = next(begin);


            while (stop_b != end) {
                if (*begin == *stop_b) {
                    break;
                }

                ++span_count;
                total_distance += catalogue_.GetDistance(*stop_a, *stop_b);

                size_t finish_id = GetStopId((*stop_b)->name);

                stop_a = stop_b;
                ++stop_b;

                if (passed.count(finish_id)) {
                    continue;
                }

                passed.emplace(finish_id);

                graph_->AddEdge({start_id, finish_id, Passage{start, bus, span_count, wait_time,
                                                            wait_time + COEFFICIENT * total_distance / velocity}
                                }
                );
            }

            ++begin;
        }
    }
