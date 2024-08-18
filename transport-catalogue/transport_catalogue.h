#pragma once

#include <deque>
#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include <set>

#include "geo.h"

using namespace std;

namespace TransportCatalogue {
	struct Stop {
		string name;
		Coordinates coordinates;
	};

	struct Bus {
		string name;
		// вектор остановок
		vector<const Stop*> bus;
	};

	// Bus X: R stops on route, U unique stops, L route length 
	struct StatBus {
		size_t stops_on_route = 0;
		size_t unique_stops = 0;
		double route_length = 0;
	};


	class TransportCatalogue {
	public:
		
		void AddBus(const Bus& bus);
		void AddStop(const Stop& stop);

		const Bus* FindBus(const string& bus_name) const;
		const Stop* FindStop(const string& stop_name) const;
		
		// Bus X: R stops on route, U unique stops, L route length 
		const optional<StatBus> GetInfoBus(const string& bus_name) const;

		// Stop X: buses bus1 bus2 ... busN 
		const optional<set<string>> GetInfoStop(const string& stop_name) const;

	private:
		//  дек остановок
		deque<Stop> stops_;
		// хеш таблица для быстрого поиска остановки по имени 
		unordered_map<string, const Stop*> stopname_to_stops_;

		// дек маршрутов	
		deque<Bus> buses_; 
		// хеш таблица для быстрого поиска маршрута по имени
		unordered_map<string, const Bus*> busname_to_bus_;

		unordered_map<string, set<string>> stop_to_buses_;
	};
}
