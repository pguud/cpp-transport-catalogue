#pragma once

#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <optional>
#include <set>

#include "geo.h"
#include "json.h"

#include <map>

#include <algorithm>
#include <cassert>

namespace TransportCatalogue {
	struct Stop {
		std::string name;
		geo::Coordinates coordinates;
	};

	struct Bus {
		std::string name;
		// вектор остановок
		std::vector<const Stop*> bus;
		bool is_roundtrip;
	};

	// Bus X: R stops on route, U unique stops, L route length, C curvature
	struct StatBus {
		int stops_on_route = 0;
		int unique_stops = 0;
		double route_length = 0;
		double curvature = 0;
	};


	class TransportCatalogue {
	public:
		
		void AddBus(const Bus& bus);
		void AddStop(const Stop& stop);

		const Bus* FindBus(const std::string_view& bus_name) const;
		const Stop* FindStop(const std::string_view& stop_name) const;
		
		// Bus X: R stops on route, U unique stops, L route length, C curvature 
		const std::optional<StatBus> GetInfoBus(const std::string_view& bus_name) const;

		// Stop X: buses bus1 bus2 ... busN 
		const std::set<std::string>* GetInfoStop(const std::string_view& stop_name) const;

		// задание дистанции между остановками.
		void SetDistanceBetweenStops(const std::string_view& first_stop_name, 
									const std::string_view& second_stop_name, double dist);
		
		// получение дистанции между остановками.
		double GetDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const;

		std::map<std::string, geo::Coordinates> GetSetStops() const {
			std::map<std::string, geo::Coordinates> set_stops;
			for (const Stop& stop: stops_) {
				if (!stop_to_buses_.at(stop.name).empty()) {
					set_stops[stop.name] = stop.coordinates;
				}
			}
			return set_stops;
		}

		std::map<std::string, std::vector<geo::Coordinates>> GetCoordinatesAllBuses() const {
			std::map<std::string, std::vector<geo::Coordinates>> coordinate_all_buses;
			
			for (const auto& bus : buses_) {
				std::vector<geo::Coordinates> coordinate_buse; 

				for (const Stop* stop : bus.bus) {
					coordinate_buse.push_back(stop->coordinates);
				}
				coordinate_all_buses[bus.name] = coordinate_buse;
			}
			return coordinate_all_buses;
		}

		/////////////////////////////////////////////////////////////////
		std::vector<const Stop*> GetActualStops() const {
			std::vector<const Stop*> result;

			for (const auto& [stop, buses]: stop_to_buses_) {
				if (!buses.empty() && (stopname_to_stops_.find(stop) != stopname_to_stops_.end())) {
					result.push_back(stopname_to_stops_.at(stop));

				}
			}

			std::sort(result.begin(), result.end(),
					[](const Stop* lhs, const Stop* rhs) {
						return lhs->name < rhs->name;
					}
			);

			

			return result;
		}

		std::vector<const Bus*> GetActualBuses() const {
			std::vector<const Bus*> result;

			for (const auto& [_, bus]: busname_to_bus_) {
				if (!bus->bus.empty()) {
					result.emplace_back(bus);
				}
			}

			std::sort(result.begin(), result.end(),
					[](const Bus* lhs, const Bus* rhs) {
						return lhs->name < rhs->name;
					}
			);

			return result;
		}

		double GetDistance(const Stop* stop_a, const Stop* stop_b) const {
			auto distance = distance_between_stops_.find(std::pair{stop_a, stop_b});
			if (distance == distance_between_stops_.end()) {
				distance = distance_between_stops_.find(std::pair{stop_b, stop_a});
			}
			assert(distance != distance_between_stops_.end());

			return distance->second;
		}

		bool IsSingleStop(std::string_view name) const {
			auto stop = stopname_to_stops_.find(std::string(name));
			assert(stop != stopname_to_stops_.end());

			return stop_to_buses_.at(std::string(name)).empty();
		}

		    const std::vector<const Stop*>& GetAllStops() const {
				static std::vector<const Stop*> stops;
				stops.clear();
				for (const auto& stop : stops_) {
					stops.push_back(&stop);
				}
				return stops;
				}

				const std::vector<const Bus*>& GetAllBuses() const {
				static std::vector<const Bus*> buses;
				buses.clear();
				for (const auto& bus : buses_) {
					buses.push_back(&bus);
				}
				return buses;
				}

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

	    std::unordered_map<const Stop*, std::unordered_set<const Bus*>> crossroads_;


		struct PairHash {
			template <typename T1, typename T2>
			std::size_t operator()(const std::pair<T1, T2>& pair) const {
				std::size_t h1 = std::hash<T1>{}(pair.first);
				std::size_t h2 = std::hash<T2>{}(pair.second);
				return h1 ^ (h2 << 1); // Комбинация хешей с помощью XOR и сдвига
			}
		};

		// Фактическое расстояние между остановками
		std::unordered_map<std::pair<const Stop*, const Stop*>, double, PairHash> distance_between_stops_;
	};
}
