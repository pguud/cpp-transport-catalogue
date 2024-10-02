#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"

int main() {
    TransportCatalogue::TransportCatalogue catalogue;

    JSONReader read{catalogue};
    
    read.ReadBaseRequests();
    json::Document doc = read.GetStatRequests();

    json::Print(doc, std::cout);
}
