#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"

class JSONReader {
public:
    explicit JSONReader(TransportCatalogue::TransportCatalogue& catalogue);

    void ReadBaseRequests();

    RenderSettings GetRenderSettings() const ;
    
    json::Document GetStatRequests() const; 

private:
    TransportCatalogue::TransportCatalogue& catalogue_;
    json::Document json_document_;

    void SetStops();
    void SetDistanceBetweenStops();
    void SetBus();

    std::string ColorToString(json::Node color) const;
};
