#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "graph.h"
#include <memory>

    struct RoutingSettings {
    int bus_wait_time = 0;  
    double bus_velocity = 0.0;
};

class TransportRouter {
public:
    

    struct RouteItem {
        std::string type; 
        std::string stop_name;
        std::string bus_name;  
        int span_count = 0;    
        double time = 0.0;
    };

    
    struct RouteInfo {
        double total_time = 0.0;
        std::vector<RouteItem> items;
    };

    TransportRouter(const TransportCatalogue::TransportCatalogue& catalogue, const RoutingSettings& routing_settings);

    std::optional<RouteInfo> FindRoute(const std::string& from, const std::string& to) const;

private:
    const TransportCatalogue::TransportCatalogue& catalogue_;
    const RoutingSettings& routing_settings_;

    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_;

    std::unordered_map<const TransportCatalogue::Stop*, graph::VertexId> stop_to_vertex_id_;

    std::unordered_map<graph::EdgeId, RouteItem> edge_id_to_route_item_;

    std::unordered_map<const TransportCatalogue::Stop*, std::pair<graph::VertexId, graph::VertexId>> stop_to_vertex_ids_;

    void BuildGraph();
};
