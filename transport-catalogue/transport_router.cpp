#include "transport_router.h"

//namespace transport_catalogue {

    TransportRouter::TransportRouter(const TransportCatalogue::TransportCatalogue& catalogue, const RoutingSettings& routing_settings)
        : catalogue_(catalogue)
        , routing_settings_(routing_settings)
    {
        BuildGraph();
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }

    void TransportRouter::BuildGraph() {
        const auto& stops = catalogue_.GetAllStops();

        size_t vertex_id = 0;
        for (const auto& stop : stops) {
            graph::VertexId wait_vertex_id = vertex_id++;
            graph::VertexId bus_vertex_id = vertex_id++;
            stop_to_vertex_ids_[stop] = { wait_vertex_id, bus_vertex_id }; // {Stop Wait, Stop}
        }

        graph_ = graph::DirectedWeightedGraph<double>(vertex_id);

        const double bus_wait_time = routing_settings_.bus_wait_time;
        const double bus_velocity = routing_settings_.bus_velocity * 1000 / 60; 

        for (const auto& stop : stops) {
            graph::Edge<double> edge_wait = {
                stop_to_vertex_ids_[stop].first,   // Stop Wait
                stop_to_vertex_ids_[stop].second,  // Stop
                bus_wait_time
            };
            auto edge_id = graph_.AddEdge(edge_wait);
            edge_id_to_route_item_[edge_id] = RouteItem{
                "Wait",
                stop->name,
                "",
                0,
                bus_wait_time
            };
        }

        const auto& buses = catalogue_.GetAllBuses();
        for (const auto& bus : buses) {
            const auto& stops = bus->bus;
            if (stops.empty()) continue;

            for (size_t i = 0; i + 1 < stops.size(); ++i) {
                double cumulative_distance = 0.0;
                int span_count = 0;

                for (size_t j = i + 1; j < stops.size(); ++j) {
                    cumulative_distance += catalogue_.GetDistanceBetweenStops(stops[j - 1], stops[j]);
                    ++span_count;

                    double travel_time = cumulative_distance / bus_velocity;

                    graph::Edge<double> edge_bus = {
                        stop_to_vertex_ids_[stops[i]].second,  // Stop òåêóùåé îñòàíîâêè
                        stop_to_vertex_ids_[stops[j]].first,   // Stop Wait ñëåäóþùåé îñòàíîâêè
                        travel_time
                    };
                    auto edge_id = graph_.AddEdge(edge_bus);
                    edge_id_to_route_item_[edge_id] = RouteItem{
                        "Bus",
                        "",
                        bus->name,
                        span_count,
                        travel_time
                    };
                }
            }

            if (!bus->is_roundtrip) {
                for (size_t i = stops.size() - 1; i > 0; --i) {
                    double cumulative_distance = 0.0;
                    int span_count = 0;

                    for (size_t j = i - 1; j < i; --j) {
                        cumulative_distance += catalogue_.GetDistanceBetweenStops(stops[j + 1], stops[j]);
                        ++span_count;

                        double travel_time = cumulative_distance / bus_velocity;

                        graph::Edge<double> edge_bus = {
                            stop_to_vertex_ids_[stops[i]].second,  
                            stop_to_vertex_ids_[stops[j]].first,  
                            travel_time
                        };
                        auto edge_id = graph_.AddEdge(edge_bus);
                        edge_id_to_route_item_[edge_id] = RouteItem{
                            "Bus",
                            "",
                            bus->name,
                            span_count,
                            travel_time
                        };
                        if (j == 0) break;
                    }
                }
            }
        }
    }


    std::optional<TransportRouter::RouteInfo> TransportRouter::FindRoute(const std::string& from, const std::string& to) const {
        const auto* stop_from = catalogue_.FindStop(from);
        const auto* stop_to = catalogue_.FindStop(to);

        if (!stop_from || !stop_to) {
            return std::nullopt;
        }

        auto route = router_->BuildRoute(
            stop_to_vertex_ids_.at(stop_from).first,  
            stop_to_vertex_ids_.at(stop_to).first      
        );
        if (!route) {
            return std::nullopt;
        }

        RouteInfo route_info;
        route_info.total_time = route->weight;

        for (const auto& edge_id : route->edges) {
            const auto& item = edge_id_to_route_item_.at(edge_id);

            route_info.items.push_back(item);
        }

        return route_info;
    }

