#include <iostream>

#include "stat_reader.h"

using namespace std;

void ParseRequest(TransportCatalogue::TransportCatalogue& catalogue, std::istream& input, std::ostream& output) {
	int stat_request_count;
    input >> stat_request_count >> ws;

    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(input, line);
        ParseAndPrintStat(catalogue, line, output);
    }
}

void ParseAndPrintStat(const TransportCatalogue::TransportCatalogue& tansport_catalogue, std::string_view request,
                        std::ostream& output) {
	
	string_view command = request.substr(0, request.find(' '));
	string_view id = request.substr(request.find(' ') + 1, request.size());
	
	if (command == "Bus"s) {
		auto stat_bus = tansport_catalogue.GetInfoBus(string(id));
		if (stat_bus.has_value()) {
			const TransportCatalogue::StatBus s = stat_bus.value();
			output << command << " "s << id << ": "s << s.stops_on_route << " stops on route, "s << s.unique_stops << " unique stops, "s
			<< s.route_length << " route length"s << endl;
		} else {
			output << command << " "s << id << ": "s << "not found" << endl;
		}
	} else if (command == "Stop"s) {
		output << command << " "s << id << ": "s;

		if (tansport_catalogue.GetInfoStop(string(id)) != nullptr) {
			const set<string>& stat_stop = *tansport_catalogue.GetInfoStop(string(id));
			if (!stat_stop.empty()) {
				output << "buses"s;
				for (const string& s : stat_stop) {
					output << " "s << s; 
				}
				output << endl;
			} else {
				output << "no buses"s << endl;
			}
			
		} else {
			output << "not found"s << endl;
		}
	}
}
