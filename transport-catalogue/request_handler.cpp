#include "request_handler.h"
 
namespace request_handler {
 
struct EdgeInfoGetter {
    
    Node operator()(const StopEdge& edge_info) {
        using namespace std::literals;
 
        return transport_catalogue::detail::json::Builder::Builder{}.StartDict()
                        .Key("type").Value("Wait")
                        .Key("stop_name").Value(std::string(edge_info.name))
                        .Key("time").Value(edge_info.time)
                        .EndDict()
                        .Build();
    }
 
    Node operator()(const BusEdge& edge_info) {
        using namespace std::literals;
    
        return transport_catalogue::detail::json::Builder::Builder{}.StartDict()
                        .Key("type").Value("Bus")
                        .Key("bus").Value(std::string(edge_info.bus_name))
                        .Key("span_count").Value(static_cast<int>(edge_info.span_count))
                        .Key("time").Value(edge_info.time)
                        .EndDict()
                        .Build();
    }
};
 
Node RequestHandler::ExecuteMakeNodeStop(int id_request, const StopQueryResult& stop_info) {
    Node result;
    Array buses;
    transport_catalogue::detail::json::Builder::Builder Builder;
 
    std::string str_not_found = "not found";
 
    if (stop_info.not_found) {
        Builder.StartDict()
               .Key("request_id").Value(id_request)
               .Key("error_message").Value(str_not_found)
               .EndDict();
        
        result = Builder.Build();
        
    } else {
        Builder.StartDict()
               .Key("request_id").Value(id_request)
               .Key("buses").StartArray();
 
        for (std::string bus_name : stop_info.buses_name) {
            Builder.Value(bus_name);
        }
 
        Builder.EndArray().EndDict();
        
        result = Builder.Build();
    }
 
    return result;
}
 
Node RequestHandler::ExecuteMakeNodeBus(int id_request, const BusQueryResult& bus_info) {
    Node result;
    std::string str_not_found = "not found";
 
    if (bus_info.not_found) {
        result = transport_catalogue::detail::json::Builder::Builder{}.StartDict()
                          .Key("request_id").Value(id_request)
                          .Key("error_message").Value(str_not_found)
                          .EndDict()
                          .Build();
    } else {
        result = transport_catalogue::detail::json::Builder::Builder{}.StartDict()
                          .Key("request_id").Value(id_request)
                          .Key("curvature").Value(bus_info.curvature)
                          .Key("route_length").Value(bus_info.route_length)
                          .Key("stop_count").Value(bus_info.stops_on_route)
                          .Key("unique_stop_count").Value(bus_info.unique_stops)
                          .EndDict()
                          .Build();
    }
 
    return result;
}
 
Node RequestHandler::ExecuteMakeNodeMap(int id_request, 
                                           TransportCatalogue& catalogue_, 
                                           RenderSettings render_settings) {
    Node result;
 
    std::ostringstream map_stream;
    std::string map_str;
 
    MapRenderer map_catalogue(render_settings);
    
    map_catalogue.InitSphereProjector(GetStopsCoordinates(catalogue_));
    
    ExecuteRenderMap(map_catalogue, catalogue_);
    map_catalogue.GetStreamMap(map_stream);
    map_str = map_stream.str();
 
    result = transport_catalogue::detail::json::Builder::Builder{}.StartDict()
                      .Key("request_id").Value(id_request)
                      .Key("map").Value(map_str)
                      .EndDict()
                      .Build();
 
    return result;
}
 
Node RequestHandler::ExecuteMakeNodeRoute(StatRequest& request, 
                                             TransportCatalogue& catalogue, 
                                             TransportRouter& routing) {
    const auto& route_info = GetRouteInfo(request.from, request.to, catalogue, routing);
 
    if (!route_info) {
        return transport_catalogue::detail::json::Builder::Builder{}.StartDict()
                        .Key("request_id").Value(request.id)
                        .Key("error_message").Value("not found")
                        .EndDict()
                        .Build();
    }
 
    Array items;
    for (const auto& item : route_info->edges) {
        items.emplace_back(std::visit(EdgeInfoGetter{}, item));
    }
 
    return transport_catalogue::detail::json::Builder::Builder{}.StartDict()
                    .Key("request_id").Value(request.id)
                    .Key("total_time").Value(route_info->total_time)
                    .Key("items").Value(items)
                    .EndDict()
                    .Build();
}
 
void RequestHandler::ExecuteQueries(TransportCatalogue& catalogue,
                                     std::vector<StatRequest>& stat_requests,
                                     RenderSettings& render_settings,
                                     RoutingSettings& routing_settings) {
 
    std::vector<Node> result_request;
    TransportRouter transport_router;
    
    transport_router.SetRoutingSettings(routing_settings);
    transport_router.BuildRouter(catalogue);
    
    for (StatRequest req : stat_requests) {
 
        if (req.type == "Stop") {
            result_request.push_back(ExecuteMakeNodeStop(req.id, StopQuery(catalogue, req.name)));
            
        } else if (req.type == "Bus") {
            result_request.push_back(ExecuteMakeNodeBus(req.id, BusQuery(catalogue, req.name)));
            
        } else if (req.type == "Map") {
            result_request.push_back(ExecuteMakeNodeMap(req.id, catalogue, render_settings));
            
        } else if (req.type == "Route") {
            result_request.push_back(ExecuteMakeNodeRoute(req, catalogue, transport_router));
        }   
    }
 
    doc_out = Document{Node(result_request)};
}
 
void RequestHandler::ExecuteRenderMap(MapRenderer& map_catalogue, TransportCatalogue& catalogue) const {
    std::vector<std::pair<Bus*, int>> buses_palette;
    std::vector<Stop*> stops_sort;
    int palette_size = 0;
    int palette_index = 0;
 
    palette_size = map_catalogue.GetPaletteSize();
    
    if (palette_size == 0) {
        std::cout << "color palette is empty";
        return;
    }
 
    auto buses = catalogue.GetBusNameToBus();
    if (buses.size() > 0) {
 
        for (std::string_view bus_name : GetSortBusesNames(catalogue)) {
            Bus* bus_info = catalogue.GetBus(bus_name);
 
            if (bus_info) {
                if (bus_info->stops.size() > 0) {
                    buses_palette.push_back(std::make_pair(bus_info, palette_index));
                    palette_index++;
 
                    if (palette_index == palette_size) {
                        palette_index = 0;
                    }
                }
            }
        }
 
        if (buses_palette.size() > 0) {
            map_catalogue.AddLine(buses_palette);
            map_catalogue.AddBusesName(buses_palette);
        }
    }
 
    auto stops = catalogue.GetStopNameToStop();
    if (stops.size() > 0) {
        std::vector<std::string_view> stops_name;
 
        for (auto& [stop_name, stop] : stops) {
 
            if (stop->buses.size() > 0) {
                stops_name.push_back(stop_name);
            }
        }
 
        std::sort(stops_name.begin(), stops_name.end());
 
        for (std::string_view stop_name : stops_name) {
            Stop* stop = catalogue.GetStop(stop_name);
            if (stop) {
                stops_sort.push_back(stop);
            }
        }
 
        if (stops_sort.size() > 0) {
            map_catalogue.AddStopsCircle(stops_sort);
            map_catalogue.AddStopsName(stops_sort);
        }
    }
}
 
std::optional<RouteInfo> RequestHandler::GetRouteInfo(std::string_view start, 
                                                        std::string_view end, 
                                                        TransportCatalogue& catalogue, 
                                                        TransportRouter& routing) const {
 
    return routing.GetRouteInfo(routing.GetRouterByStop(catalogue.GetStop(start))->bus_wait_start, 
                                  routing.GetRouterByStop(catalogue.GetStop(end))->bus_wait_start);
}
 
std::vector<geo::Coordinates> RequestHandler::GetStopsCoordinates(TransportCatalogue& catalogue_) const {
 
    std::vector<geo::Coordinates> stops_coordinates;
    auto buses = catalogue_.GetBusNameToBus();
 
    for (auto& [busname, bus] : buses) {
        
        for (auto& stop : bus->stops) {
            geo::Coordinates coordinates;
            coordinates.latitude = stop->latitude;
            coordinates.longitude = stop->longitude;
 
            stops_coordinates.push_back(coordinates);
        }
    }
    return stops_coordinates;
}
 
std::vector<std::string_view> RequestHandler::GetSortBusesNames(TransportCatalogue& catalogue_) const {
    std::vector<std::string_view> buses_names;
 
    auto buses = catalogue_.GetBusNameToBus();
    if (buses.size() > 0) {
 
        for (auto& [busname, bus] : buses) {
            buses_names.push_back(busname);
        }
 
        std::sort(buses_names.begin(), buses_names.end());
 
        return buses_names;
 
    } else {
        return {};
    }
}
 
BusQueryResult RequestHandler::BusQuery(TransportCatalogue& catalogue, std::string_view bus_name) {
    BusQueryResult bus_info;
    Bus* bus = catalogue.GetBus(bus_name);
 
    if (bus != nullptr) {
        bus_info.name = bus->name;
        bus_info.not_found = false;
        bus_info.stops_on_route = static_cast<int>(bus->stops.size());
        bus_info.unique_stops = static_cast<int>(catalogue.GetUniqStops(bus).size());
        bus_info.route_length = static_cast<int>(bus->route_length);
        bus_info.curvature = double(catalogue.GetDistanceToBus(bus)
                                   /catalogue.GetLength(bus));
    } else {
        bus_info.name = bus_name;
        bus_info.not_found = true;
    }
 
    return bus_info;
}
 
StopQueryResult RequestHandler::StopQuery(TransportCatalogue& catalogue, std::string_view stop_name) {
    std::unordered_set<const Bus*> unique_buses;
    StopQueryResult stop_info;
    Stop* stop = catalogue.GetStop(stop_name);
 
    if (stop != NULL) {
 
        stop_info.name = stop->name;
        stop_info.not_found = false;
        unique_buses = catalogue.GetStopUniqBuses(stop);
 
        if (unique_buses.size() > 0) {
            
            for (const Bus* bus : unique_buses) {
                stop_info.buses_name.push_back(bus->name);
            }
 
            std::sort(stop_info.buses_name.begin(), stop_info.buses_name.end());
        }
        
    } else {
        stop_info.name = stop_name;
        stop_info.not_found = true;
    }
 
    return stop_info;
}
 
const Document& RequestHandler::GetDocument() {
    return doc_out;
}
 
} // namespace request_handler
