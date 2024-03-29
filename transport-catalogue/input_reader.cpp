#include "input_reader.h"

#include <stdlib.h>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <charconv>
#include <iterator>

#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue::input_reader {

    namespace from_char_stream {

        namespace detail {
            // делит строку на части разделённые строкой `by`
            vector<string_view> SplitNoWS(string_view line, const string_view by) {
                vector<string_view> parts;
                do {
                    size_t begin = 0;
                    for (; begin < line.size() && isspace(line[begin]); ++begin)
                        ;

                    size_t delim_i = min(line.find(by, begin), line.size());
                    size_t end = delim_i;
                    for (; end > begin && isspace(line[end - 1]); --end)
                        ;
                    
                    if (by.size() == 1 && isspace(by[0]) && delim_i == line.size() &&
                        end == begin) {
                    }
                    
                    else {
                        parts.push_back(line.substr(begin, end - begin));
                    }
                    
                    if (delim_i == line.size() - by.size()) {
                        parts.push_back(line.substr(line.size(), 0));
                    }

                    line.remove_prefix(min(delim_i + by.size(), line.size()));
                } while (line.size() > 0);
                return parts;
            }

            // делит строку на части разделённые символом `by`
            vector<string_view> SplitNoWS(string_view line, char by) {
                return SplitNoWS(line, string_view{ &by, 1 });
            }

            // парсит команду на добавление остановки
            AddStopCmd ParseAddStopCmd(string_view line) {
                auto name_rest = SplitNoWS(line, ':');
                assert(name_rest.size() == 2);
                auto coords_distances = SplitNoWS(name_rest[1], ',');
                assert(coords_distances.size() >= 2);
                char* endp = nullptr;
                double lat = strtod(coords_distances[0].data(), &endp);
                double lng = strtod(coords_distances[1].data(), &endp);
                vector<AddStopCmd::Distance> distances;
                distances.reserve(coords_distances.size() - 2);
                for (size_t i = 2; i < coords_distances.size(); ++i) {
                    auto dis_parts = SplitNoWS(coords_distances[i], " to "sv);
                    assert(dis_parts.size() == 2);
                    auto len_part = dis_parts[0];
                    assert(len_part.size() > 0 && len_part.back() == 'm');
                    size_t dis = 0;
                    from_chars(len_part.data(), len_part.data() + len_part.size() - 1, dis);
                    distances.emplace_back(string{ dis_parts[1] }, dis);
                }
                return { string{name_rest[0]}, {lat, lng}, move(distances) };
            }

            // парсит команду на добавление маршрута
            AddBusCmd ParseAddBusCmd(string_view line) {
                auto name_route = SplitNoWS(line, ':');
                assert(name_route.size() == 2);
                string_view name = name_route[0];
                string_view route = name_route[1];
                size_t marker_pos = route.find_first_of(">-"sv);
                assert(marker_pos != route.npos);
                char marker = route[marker_pos];
                RouteType route_type =
                    marker == '-' ? RouteType::LINEAR : RouteType::CIRCULAR;
                auto stops_sv = SplitNoWS(route, marker);
                vector<string> stops;
                stops.reserve(stops_sv.size());
                transform(stops_sv.begin(), stops_sv.end(), back_inserter(stops),
                    [](string_view sv) { return string{ sv }; });

                return {
                    string{name},
                    route_type,
                    move(stops),
                };
            }

        }  // namespace detail

        // парсит команды из символьного потока и складывает результат 
        // в векторы `add_stop_cmds_` и `add_bus_cmds_`
        void DbReader::Parse() {
            using detail::ParseAddBusCmd;
            using detail::ParseAddStopCmd;

            int cmd_count;
            sin_ >> cmd_count;
            while (cmd_count--) {
                string cmd;
                sin_ >> cmd;
                if (cmd == "Stop"s) {
                    string line;
                    getline(sin_ >> ws, line);
                    add_stop_cmds_.push_back(ParseAddStopCmd(string_view{ line }));
                }
                else if (cmd == "Bus"s) {
                    string line;
                    getline(sin_ >> ws, line);
                    add_bus_cmds_.push_back(ParseAddBusCmd(string_view{ line }));
                }
            }
        }

        // прочитать данные для транспортного справочника из символьного потока
        void ReadDB(TransportCatalogue& transport_catalogue, std::istream& sin) {
            DbReader input{ sin };
            for (const AddStopCmd& add_stop : input.GetAddStopCmds()) {
                transport_catalogue.AddStop(add_stop.name, add_stop.coordinates);
            }
            for (const AddStopCmd& add_stop : input.GetAddStopCmds()) {
                for (auto distance_pair : add_stop.distances) {
                    transport_catalogue.SetDistance(add_stop.name, distance_pair.first,
                        distance_pair.second);
                }
            }
            for (const AddBusCmd& bus : input.GetAddBusCmds()) {
                transport_catalogue.AddBus(bus.name, bus.route_type, bus.stop_names);
            }
        }

    }  // namespace from_char_stream

}  // namespace transport_catalogue::input_reader
