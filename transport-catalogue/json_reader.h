#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"

#include "transport_router.h"

class JSONReader {
public:
    explicit JSONReader(TransportCatalogue::TransportCatalogue& catalogue);

    void ReadBaseRequests();

    RenderSettings GetRenderSettings() const ;
    
    json::Document GetStatRequests() const; 

private:
    TransportCatalogue::TransportCatalogue& catalogue_;
    json::Document json_document_;
    RoutingSettings routing_settings_;


    void SetStops();
    void SetDistanceBetweenStops();
    void SetBus();
    
    // Во входной JSON добавляется ключ routing_settings, значение которого — словарь с двумя ключами
    void SetRoutingSettings();

    std::string ColorToString(json::Node color) const;
};
