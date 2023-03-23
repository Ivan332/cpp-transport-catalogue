#include "transport_catalogue.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <set>
#include <stdexcept>
#include <unordered_set>

#include "geo.h"

using namespace std;

namespace transport_catalogue {

    // добавить остановку в транспортный справочник
    void TransportCatalogue::AddStop(string name, geo::Coordinates coordinates) {
        if (stops_by_name_.count(name) > 0) {
            throw invalid_argument("stop "s + name + " already exists"s);
        }
        auto& ref = stops_.emplace_back(Stop{ move(name), coordinates });
        stops_by_name_.emplace(string_view{ ref.name }, &ref);
    }

    // добавить маршрут в транспортный справочник
    void TransportCatalogue::AddBus(string name, RouteType route_type,
        const vector<string>& stop_names) {
        if (buses_by_name_.count(name) > 0) {
            throw invalid_argument("bus "s + name + " already exists"s);
        }
        if (stop_names.size() == 0) {
            throw invalid_argument("empty stop list"s);
        }
        if (route_type == RouteType::CIRCULAR && stop_names[0] != stop_names.back()) {
            throw invalid_argument(
                "first and last stop in circular routes must be the same"s);
        }
        vector<const Stop*> stops = ResolveStopNames(stop_names);

        if (route_type == RouteType::CIRCULAR) {
            stops.resize(stops.size() - 1);
        }

        const auto& ref =
            buses_.emplace_back(Bus{ move(name), route_type, move(stops) });
        buses_by_name_.emplace(ref.name, &ref);
        for (const Stop* stop : ref.stops) {
            auto& buses_for_stop = buses_for_stop_[stop];
            buses_for_stop.emplace(ref.name);
        }
    }

    // переводит вектор с названиями остановок в вектор с указателями на остановку в справочнике
    vector<const Stop*> TransportCatalogue::ResolveStopNames(
        const vector<string>& stop_names) {
        vector<const Stop*> stops;
        stops.reserve(stop_names.size());
        for (const auto& stop_name : stop_names) {
            auto found_it = stops_by_name_.find(stop_name);
            if (found_it == stops_by_name_.end()) {
                throw invalid_argument("unknown bus stop "s + stop_name);
            }
            stops.push_back(found_it->second);
        }
        return stops;
    }

    // получить информацию о маршруте
    optional<BusStats> TransportCatalogue::GetBusStats(string_view bus_name) const {
        auto it = buses_by_name_.find(bus_name);
        if (it == buses_by_name_.end()) {
            return nullopt;
        }
        const Bus& bus = *it->second;
        const auto& stops = bus.stops;
      
        unordered_set<const Stop*> uniq_stops{ stops.begin(), stops.end() };

        size_t stops_count;
        double route_length = 0;
        double crow_route_length = 0;
        assert(stops.size() > 0);
        for (size_t i = 1; i < stops.size(); ++i) {
            auto [real, crow] = CalcDistance(stops[i - 1], stops[i]);
            route_length += real;
            crow_route_length += crow;
        }
        switch (bus.route_type) {
        case RouteType::LINEAR:
            stops_count = stops.size() * 2 - 1;
            
            for (size_t i = stops.size() - 1; i >= 1; --i) {
                auto [real, _] = CalcDistance(stops[i], stops[i - 1]);
                route_length += real;
            }

            crow_route_length *= 2;
            break;
        case RouteType::CIRCULAR:
            stops_count = stops.size() + 1;
            
            auto [real, crow] = CalcDistance(stops.back(), stops[0]);
            route_length += real;
            crow_route_length += crow;
            break;
        }
        return BusStats{ stops_count, uniq_stops.size(), route_length,
                        crow_route_length };
    }

    // получить информацию об остановке
    std::optional<BusesForStop> TransportCatalogue::GetStopInfo(
        std::string_view stop_name) const {
        auto stop_it = stops_by_name_.find(stop_name);
        if (stop_it == stops_by_name_.end()) {
            return nullopt;
        }
        auto buses_it = buses_for_stop_.find(stop_it->second);

        if (buses_it == buses_for_stop_.end()) {
            return BusesForStop{};
        }
        return buses_it->second;
    }

    // задать реальное расстояние от остановки `from` до `to`
    void TransportCatalogue::SetDistance(std::string_view from, std::string_view to,
        size_t distance) {
        auto from_it = stops_by_name_.find(from);
        if (from_it == stops_by_name_.end()) {
            throw invalid_argument{ "unknown stop "s + string(from) };
        }
        auto to_it = stops_by_name_.find(to);
        if (to_it == stops_by_name_.end()) {
            throw invalid_argument{ "unknown stop "s + string(to) };
        }
        detail::StopDisKey key{ from_it->second, to_it->second };
        if (real_distances_.count(key) > 0) {
            throw invalid_argument{ "distance between "s + string(from) + " and "s +
                                   string(to) + " has already been set" };
        }
        real_distances_.emplace(key, distance);
    }

    // рассчитать расстояние от остановки `from` до `to`
    pair<double, double> TransportCatalogue::CalcDistance(const Stop* from,
        const Stop* to) const {
        // as the crow flies
        double crow_dis = geo::ComputeDistance(from->coords, to->coords);
        double real_dis = crow_dis;
        auto it = real_distances_.find({ from, to });
        if (it == real_distances_.end()) {
            it = real_distances_.find({ to, from });
        }
        if (it != real_distances_.end()) {
            real_dis = it->second;
        }
        return { real_dis, crow_dis };
    }

}  // namespace transport_catalogue
