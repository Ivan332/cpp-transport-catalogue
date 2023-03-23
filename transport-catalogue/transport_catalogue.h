#pragma once

#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <optional>
#include <unordered_map>
#include <utility>

#include "geo.h"

namespace transport_catalogue {
	// тип маршрута (линейный / круговой)
	enum RouteType {		
		LINEAR,
		CIRCULAR,
	};

	// остановка(название / координаты)
	struct Stop {
		std::string name;
		geo::Coordinates coords;
	};

	// маршрут(название / тип маршрута / вектор остановок)
	struct Bus {
		std::string name;
		RouteType route_type = RouteType::LINEAR;
		std::vector<const Stop*> stops;
	};

	// параметры маршрута
	struct BusStats {
		size_t stops_count = 0;
		size_t unique_stops_count = 0;
		double route_length = 0;
		double crow_route_length = 0;
	};

	using BusesForStop = std::set<std::string_view>;

	namespace detail {
		// ключ для мапы с расстояниями между остановками
		using StopDisKey = std::pair<const Stop*, const Stop*>;
		
		// хешер для `StopDisKey`
		struct SDKHasher {
			size_t operator()(const StopDisKey sdk) const {
				return reinterpret_cast<size_t>(sdk.first) +
					reinterpret_cast<size_t>(sdk.second) * 43;
			}
		};

	}  // namespace detail

	class TransportCatalogue {
	public:
		void AddStop(std::string name, geo::Coordinates);

		void AddBus(std::string name, RouteType route_type,
			const std::vector<std::string>& stop_names);

		void SetDistance(std::string_view from, std::string_view to, size_t distance);

		std::optional<BusStats> GetBusStats(std::string_view bus_name) const;

		std::optional<BusesForStop> GetStopInfo(std::string_view stop_name) const;

		//std::vector<const Bus*> GetBuses() const;

		//std::vector<const Stop*> GetStops() const;

		//double GetRealDistance(const Stop* from, const Stop* to) const;

	private:
		// коллекция уникальных остановок
		std::deque<Stop> stops_;		

		// мапа <имя остановки> -> <указатель на остановку>
		std::unordered_map<std::string_view, Stop*> stops_by_name_;

		// коллекция уникальных маршрутов
		std::deque<Bus> buses_;		

		// мапа <имя маршрута> -> <указатель на маршрут>
		std::unordered_map<std::string_view, const Bus*> buses_by_name_;

		// мапа с реальным расстоянием между остановками `first` и `second`
		std::unordered_map<detail::StopDisKey, unsigned int, detail::SDKHasher>
			real_distances_;

		// мапа с набором маршрутов, проходящих через определённую остановку
		std::unordered_map<const Stop*, BusesForStop> buses_for_stop_;

		// вычисление дистанции от `from` до `to` 
		std::pair<double, double> CalcDistance(const Stop* from,
			const Stop* to) const;

		// переводит вектор с названиями остановок в вектор с указателями на остановку в справочнике
		std::vector<const Stop*> ResolveStopNames(
			const std::vector<std::string>& stop_names);
	};

}
