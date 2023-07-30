#pragma once

#include <iostream>

#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"
#include "svg.pb.h"
#include "map_renderer.h"
#include "map_renderer.pb.h"
#include "transport_router.h"
#include "transport_router.pb.h"
 
namespace serialization {
    
struct SerializationSettings {
    std::string file_name;
};
    
struct Catalogue {
    transport_catalogue::TransportCatalogue transport_catalogue_;
    map_renderer::RenderSettings render_settings_;
    domain::RoutingSettings routing_settings_;
};
    
template <typename It>
uint32_t calculate_id(It start, It end, std::string_view name);
    
transport_catalogue_protobuf::TransportCatalogue SerializationTransportCatalogue(const transport_catalogue::TransportCatalogue& transport_catalogue);
transport_catalogue::TransportCatalogue DeserializationTransportCatalogue(const transport_catalogue_protobuf::TransportCatalogue& transport_catalogue_proto);
 
transport_catalogue_protobuf::Color SerializationColor(const svg::Color& tc_color);
svg::Color DeserializationColor(const transport_catalogue_protobuf::Color& color_proto);
transport_catalogue_protobuf::RenderSettings SerializationRenderSettings(const map_renderer::RenderSettings& render_settings);
map_renderer::RenderSettings DeserializationRenderSettings(const transport_catalogue_protobuf::RenderSettings& render_settings_proto);
    
transport_catalogue_protobuf::RoutingSettings SerializationRoutingSettings(const domain::RoutingSettings& routing_settings);
domain::RoutingSettings DeserializationRoutingSettings(const transport_catalogue_protobuf::RoutingSettings& routing_settings_proto);
 
void SerializationCatalogue(const transport_catalogue::TransportCatalogue& transport_catalogue, 
                             const map_renderer::RenderSettings& render_settings,
                             const domain::RoutingSettings& routing_settings,
                             std::ostream& out); 
    
Catalogue DeserializationCatalogue(std::istream& in);
    
} // namespace serialization
