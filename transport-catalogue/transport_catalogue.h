#pragma once

#include <deque>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <numeric>
 
#include "domain.h"
 
using namespace domain;
 
namespace transport_catalogue {   
 
struct DistanceHasher {
    std::hash<const void*> hasher;
    
    std::size_t operator()(const std::pair<const Stop*, const Stop*> pair_stops) const noexcept {
        auto hash_1 = static_cast<const void*>(pair_stops.first);
        auto hash_2 = static_cast<const void*>(pair_stops.second);
        return hasher(hash_1) * 17 + hasher(hash_2);
    }  
};
    
typedef  std::unordered_map<std::string_view, Stop*> StopMap;
typedef  std::unordered_map<std::string_view, Bus*> BusMap;
typedef  std::unordered_map<std::pair<const Stop*, const  Stop*>, int, DistanceHasher> DistanceMap;
 
class TransportCatalogue {
public:      
    void AddBus(Bus&& bus);
    void AddStop(Stop&& stop);
    void AddDistance(const std::vector<Distance>& distances);
    
    Bus* GetBus(std::string_view bus_name);
    Stop* GetStop(std::string_view stop_name);
    
    std::deque<Stop> GetStops() const;
    std::deque<Bus> GetBuses() const;
    
    BusMap GetBusNameToBus() const;
    StopMap GetStopNameToStop() const;
    
    std::unordered_set<const Bus*> GetStopUniqBuses(Stop* stop);    
    std::unordered_set<const Stop*> GetUniqStops(Bus* bus);
    double GetLength(Bus* bus);
    
    DistanceMap GetDistance() const;
    size_t GetDistanceStop(const Stop* start, const Stop* finish) const;
    size_t GetDistanceToBus(Bus* bus);
    
private:    
    std::deque<Stop> stops;
    StopMap stopname_to_stop;
    
    std::deque<Bus> buses;
    BusMap busname_to_bus;
    
    DistanceMap distance_to_stop;
};
    
} // namespace transport_catalogue
