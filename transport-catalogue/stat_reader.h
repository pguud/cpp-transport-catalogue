#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

void ParseRequest(TransportCatalogue::TransportCatalogue& catalogue, std::istream& input);

void ParseAndPrintStat(const TransportCatalogue::TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output);
                       