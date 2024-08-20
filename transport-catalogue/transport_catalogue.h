#pragma once

#include <deque>
#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include <set>

#include "geo.h"

namespace TransportCatalogue {
	struct Stop {
		std::string name;
		Coordinates coordinates;
	};

	struct Bus {
		std::string name;
		// вектор остановок
		std::vector<const Stop*> bus;
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

		const Bus* FindBus(const std::string_view& bus_name) const;
		const Stop* FindStop(const std::string_view& stop_name) const;
		
		// Bus X: R stops on route, U unique stops, L route length 
		const std::optional<StatBus> GetInfoBus(const std::string_view& bus_name) const;

		// Stop X: buses bus1 bus2 ... busN 
		const std::set<std::string>* GetInfoStop(const std::string_view& stop_name) const;

	private:
		//  дек остановок
		std::deque<Stop> stops_;
		// хеш таблица для быстрого поиска остановки по имени 
		std::unordered_map<std::string, const Stop*> stopname_to_stops_;

		// дек маршрутов	
		std::deque<Bus> buses_; 
		// хеш таблица для быстрого поиска маршрута по имени
		std::unordered_map<std::string, const Bus*> busname_to_bus_;

		std::unordered_map<std::string, std::set<std::string>> stop_to_buses_;
	};
}
