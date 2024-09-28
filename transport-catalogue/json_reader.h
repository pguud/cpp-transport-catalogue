#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"

void SetBaseRequestsInTransportCatalogue(TransportCatalogue::TransportCatalogue& catalogue, const json::Document& doc);
json::Document SetStateRequestsInDocument(const TransportCatalogue::TransportCatalogue& catalogue, const json::Document& doc);
