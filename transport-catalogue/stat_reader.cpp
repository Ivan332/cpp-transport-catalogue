#include "stat_reader.h"

#include <stddef.h>

#include <cassert>
#include <iomanip>
#include <set>
#include <string>
#include <iostream>
#include <sstream>

#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue::stat_reader {

    namespace from_char_stream {

        namespace detail {

            // удаляет пробельные символы в конце строки
            void RTrimStr(string& str) {
                size_t last_nonws = str.find_last_not_of(" \t\r\n");
                if (last_nonws != str.npos) {
                    str.resize(last_nonws + 1);
                }
            }

            // делает из строки запрос(тип команды / имя команды)
            pair<string, string> SplitIntoCommand(const string& text) {
                auto it = text.find_first_of(' ');
                string type = text.substr(0, it);
                string name = text.substr(it + 1, text.size() - it);
                from_char_stream::detail::RTrimStr(name);

                return make_pair(type, name);
            }

        }  // namespace detail

        //// парсит запросы и выводит результат
        //void GetInfo(const TransportCatalogue& transport_catalogue) {
        //    int req_count;
        //    cin >> req_count;
        //    vector<string> requests;
        //    do {
        //        string reqest;
        //        getline(cin, reqest);
        //        if (!reqest.empty()) {
        //            requests.push_back(reqest);
        //        }
        //    } while (req_count--);
        //    for (string& reqest : requests)
        //    {
        //        auto cmd = from_char_stream::detail::SplitIntoCommand(reqest);
        //        if (cmd.first == "Bus"s) {
        //            string bus_name = cmd.second;
        //            const auto bus_stats = transport_catalogue.GetBusStats(bus_name);
        //            cout << "Bus " << bus_name << ": ";
        //            if (bus_stats.has_value()) {
        //                assert(bus_stats->crow_route_length > 0);
        //                auto old_precision = cout.precision();
        //                cout << setprecision(6) << bus_stats->stops_count << " stops on route, "
        //                    << bus_stats->unique_stops_count << " unique stops, "
        //                    << bus_stats->route_length << " route length, "
        //                    << (bus_stats->route_length / bus_stats->crow_route_length)
        //                    << " curvature" << setprecision(old_precision) << endl;
        //            }
        //            else {
        //                cout << "not found" << endl;
        //            }
        //        }
        //        if (cmd.first == "Stop"s) {
        //            string stop_name = cmd.second;
        //            const auto buses_for_stop = transport_catalogue.GetStopInfo(stop_name);
        //            cout << "Stop " << stop_name << ": ";
        //            if (!buses_for_stop.has_value()) {
        //                cout << "not found" << endl;
        //                continue;
        //                //return;
        //            }
        //            if (buses_for_stop->size() == 0) {
        //                cout << "no buses" << endl;
        //                continue;
        //                //return;
        //            }
        //            cout << "buses";
        //            for (const auto bus_name : *buses_for_stop) {
        //                cout << ' ' << bus_name;
        //            }
        //            cout << endl;
        //        }
        //    }
        //}

        // парсит запросы на получение статистики из потока и печатает результат
        void StatsRequestProcessor::ProcessRequests(
            const TransportCatalogue& transport_catalogue,
            to_char_stream::StatsPrinter& stats_printer) {
            int req_count;
            cin >> req_count;
            vector<string> requests;
            do {
                string reqest;
                getline(cin, reqest);
                if (!reqest.empty()) {
                    requests.push_back(reqest);
                }
            } while (req_count--);

            for (string& reqest : requests) {
                auto cmd = from_char_stream::detail::SplitIntoCommand(reqest);
                if (cmd.first == "Bus"s) {
                    const auto bus_stats = transport_catalogue.GetBusStats(cmd.second);
                    stats_printer.PrintBusStats(cmd.second, bus_stats);
                }
                if (cmd.first == "Stop"s) {
                    const auto buses_for_stop = transport_catalogue.GetStopInfo(cmd.second);
                    stats_printer.PrintStopInfo(cmd.second, buses_for_stop);
                }
            }
        }

    }  // namespace from_char_stream

    namespace to_char_stream {

        // печатает статистику машрута(кол-во остановок / кол-во уникальных остановок / длину маршрута)
        void StatsPrinter::PrintBusStats(string_view bus_name,
            const optional<BusStats>& bus_stats) {
            sout_ << "Bus " << bus_name << ": ";
            if (bus_stats.has_value()) {
                assert(bus_stats->crow_route_length > 0);
                auto old_precision = sout_.precision();
                sout_ << setprecision(6) << bus_stats->stops_count << " stops on route, "
                    << bus_stats->unique_stops_count << " unique stops, "
                    << bus_stats->route_length << " route length, "
                    << (bus_stats->route_length / bus_stats->crow_route_length)
                    << " curvature" << setprecision(old_precision) << endl;
            }
            else {
                sout_ << "not found" << endl;
            }
        }

        // печатает информацию о остановке(сет маршрутов)
        void StatsPrinter::PrintStopInfo(
            std::string_view stop_name,
            const std::optional<BusesForStop>& buses_for_stop) {
            sout_ << "Stop " << stop_name << ": ";
            if (!buses_for_stop.has_value()) {
                sout_ << "not found" << endl;
                return;
            }
            if (buses_for_stop->size() == 0) {
                sout_ << "no buses" << endl;
                return;
            }
            sout_ << "buses";
            for (const auto bus_name : *buses_for_stop) {
                sout_ << ' ' << bus_name;
            }
            sout_ << endl;
        }

    }  // namespace to_char_stream

}  // namespace transport_catalogue::stat_reader
