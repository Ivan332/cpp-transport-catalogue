#include "json_reader.h"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>

#include "domain.h"
#include "json.h"
#include "svg.h"

using namespace std;

namespace json_reader {

    namespace detail {

        using namespace request_handler;

        // Парсит команду на добавление остановки 
        AddStopCmd ParseStopCmd(const json::Dict& request) {
            vector<AddStopCmd::Distance> distances;

            for (const auto& [stop_name, node] : request.at("road_distances"s).AsMap()) {
                distances.emplace_back(stop_name, node.AsInt());
            }

            return {
                request.at("name"s).AsString(),
                {request.at("latitude"s).AsDouble(), request.at("longitude"s).AsDouble()},
                move(distances),
            };
        }

        // Парсит команду на добавление маршрута
        AddBusCmd ParseBusCmd(const json::Dict& request) {
            vector<string> stop_names;

            for (const auto& node : request.at("stops"s).AsArray()) {
                stop_names.emplace_back(node.AsString());
            }

            return {
                request.at("name"s).AsString(),
                request.at("is_roundtrip"s).AsBool() ? transport_catalogue::RouteType::CIRCULAR
                                                     : transport_catalogue::RouteType::LINEAR,
                move(stop_names),
            };
        }

        // Парсит базу запросов 
        vector<BaseRequest> ParseBaseRequests(const json::Array& base_requests) {
            vector<BaseRequest> result;

            for (const auto& node : base_requests) {
                const auto& request = node.AsMap();
                const auto& type = request.at("type"s).AsString();

                if (type == "Stop"s) {
                    result.emplace_back(ParseStopCmd(request));
                }
                else if (type == "Bus"s) {
                    result.emplace_back(ParseBusCmd(request));
                }
                else {
                    throw invalid_argument("Unknown base request with type '"s + type + "'"s);
                }
            }

            return result;
        }

        StopStatRequest ParseStopStatRequest(const json::Dict& request) {
            return { request.at("id"s).AsInt(), request.at("name"s).AsString() };
        }

        BusStatRequest ParseBusStatRequest(const json::Dict& request) {
            return { request.at("id"s).AsInt(), request.at("name"s).AsString() };
        }

        vector<StatRequest> ParseStatRequests(const json::Array& stat_requests) {
            vector<StatRequest> result;

            for (const auto& node : stat_requests) {
                const auto& request = node.AsMap();
                const auto& type = request.at("type"s).AsString();

                if (type == "Stop"s) {
                    result.emplace_back(ParseStopStatRequest(request));
                }
                else if (type == "Bus"s) {
                    result.emplace_back(ParseBusStatRequest(request));
                }
                else if (type == "Map"s) {
                    result.emplace_back(MapRequest{ request.at("id"s).AsInt() });
                }
                else {
                    throw invalid_argument("Unknown stat request with type '"s + type + "'"s);
                }
            }

            return result;
        } 

        // Принтер разных вариантов овтетов на запросы статистики
        struct ResponseVariantPrinter {
            int request_id;
            ostream& out;

            void operator()(std::monostate) {
                json::Print(json::Document(json::Dict{ { "request_id"s, request_id },
                    {"error_message"s, "not found"s} }), out);
            }

            void operator()(const StopStatResponse& response) {
                json::Array array_;
                for (const auto& bus_name : response.buses_for_stop) {
                    array_.push_back(json::Node{ bus_name.data() });
                }
                std::sort(array_.begin(), array_.end(), [](const json::Node& lhs, const json::Node& rhs)
                    {return lhs.AsString() < rhs.AsString(); });
                json::Print(json::Document(json::Dict{ { "buses"s, array_ }, { "request_id"s, request_id } }), out);
            }

            void operator()(const BusStatResponse& response) {
                const auto& bus_stats = response.bus_stats;
                json::Print(json::Document(json::Dict{ { "curvature"s, bus_stats.route_length / bus_stats.crow_route_length },
                    { "request_id"s, request_id }, { "route_length"s, bus_stats.route_length },
                    { "stop_count"s, static_cast<int>(bus_stats.stops_count) },
                    { "unique_stop_count"s, static_cast<int>(bus_stats.unique_stops_count) } }), out);
            }

            void operator()(const MapResponse& response) {
                json::Print(json::Document(json::Dict{ { "map"s, response.svg_map }, { "request_id"s, request_id } }), out);
            }
        };

        // Парсит координату в JSON формате (массив из двух чисел с плавающей точкой)
        svg::Point ParsePoint(const json::Array& arr) {
            if (arr.size() == 2) {
                return { arr[0].AsDouble(), arr[1].AsDouble() };
            }
            throw runtime_error(
                "Error parsing JSON array as an SVG point. It must have 2 elements"s);
        }

        // Парсит SVG цвет в JSON формате
        svg::Color ParseColor(const json::Node& node) {
            if (node.IsString()) {
                return node.AsString();
            }
            if (node.IsArray()) {
                const auto& arr = node.AsArray();
                if (arr.size() == 3) {
                    return svg::Rgb{ static_cast<unsigned int>(arr[0].AsInt()),
                                    static_cast<unsigned int>(arr[1].AsInt()),
                                    static_cast<unsigned int>(arr[2].AsInt()) };
                }
                else if (arr.size() == 4) {
                    return svg::Rgba{ static_cast<unsigned int>(arr[0].AsInt()),
                                        static_cast<unsigned int>(arr[1].AsInt()),
                                        static_cast<unsigned int>(arr[2].AsInt()),
                                        arr[3].AsDouble() };
                }
                throw runtime_error(
                    "Error parsing JSON array as a color. It must have 3 or 4 elements"s);
            }
            throw runtime_error(
                "Error parsing JSON node as color. It can be an array or a string"s);
        }

        // Парсит настройки отрисовки карты в SVG формате
        RenderSettings ParseRenderSettings(const json::Dict& rs) {
            RenderSettings result;
            result.bus_label_font_size = rs.at("bus_label_font_size"s).AsInt();
            result.bus_label_offset = ParsePoint(rs.at("bus_label_offset"s).AsArray());
            for (const auto& node : rs.at("color_palette"s).AsArray()) {
                result.color_palette.emplace_back(ParseColor(node));
            }
            result.height = rs.at("height"s).AsDouble();
            result.line_width = rs.at("line_width"s).AsDouble();
            result.padding = rs.at("padding"s).AsDouble();
            result.stop_label_font_size = rs.at("stop_label_font_size"s).AsInt();
            result.stop_label_offset = ParsePoint(rs.at("stop_label_offset"s).AsArray());
            result.stop_radius = rs.at("stop_radius"s).AsDouble();
            result.underlayer_color = ParseColor(rs.at("underlayer_color"s));
            result.underlayer_width = rs.at("underlayer_width"s).AsDouble();
            result.width = rs.at("width"s).AsDouble();
            return result;
        }

    }  // namespace detail

    // Парсит запросы к транспортному справочнику в JSON формате
    void BufferingRequestReader::Parse(istream& sin) {
        const auto document = json::Load(sin);
        const auto& root = document.GetRoot().AsMap();
        base_requests_ =
            detail::ParseBaseRequests(root.at("base_requests"s).AsArray());
        stat_requests_ =
            detail::ParseStatRequests(root.at("stat_requests"s).AsArray());
        if (root.count("render_settings"s) > 0) {
            render_settings_ =
            detail::ParseRenderSettings(root.at("render_settings"s).AsMap());
        }
    }

    ResponsePrinter::ResponsePrinter(std::ostream& out) : out_(out) {}

    // Выводит в поток результаты запросов
    void ResponsePrinter::PrintResponse(int request_id,
        const request_handler::StatResponse& response) {
        if (printed_something_) {
            out_.put(',');
            out_.put('\n');
        }
        else {
            Begin();
        }
        detail::ResponseVariantPrinter printer{ request_id, out_ };
        std::visit(printer, response);
        printed_something_ = true;
    }

    ResponsePrinter::~ResponsePrinter() {
        if (printed_something_) {
            End();
        }
    }

    // Вставляет в начало вывода `[` 
    void ResponsePrinter::Begin() { out_.put('['); out_.put('\n');  }

    // Вставляет в конец вывода `]` 
    void ResponsePrinter::End() { out_.put('\n');  out_.put(']'); }

}  // namespace json_reader
