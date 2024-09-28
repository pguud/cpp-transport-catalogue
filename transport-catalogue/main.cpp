#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"

int main() {
    json::Document doc = json::Load(std::cin);

    TransportCatalogue::TransportCatalogue catalogue;
    SetBaseRequestsInTransportCatalogue(catalogue, doc);

    //RenderSettings render_settings;
    //SetRenderSettings(render_settings, doc);

    //json::Print(doc, std::cout);

    //SetStateRequestsInDocument(catalogue, doc);
    json::Print(SetStateRequestsInDocument(catalogue, doc), std::cout);

    //DrawMap(catalogue, render_settings);
}
