#pragma once

#include <stddef.h>

#include <istream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "transport_catalogue.h"
#include "geo.h"

namespace transport_catalogue::input_reader {
    // команда на добавление остановки в транспортный справочник
    struct AddStopCmd {
        // дистанция (название остановки / расстояние до неё)
        using Distance = std::pair<std::string, size_t>;

        // имя остановки
        std::string name;

        // координаты остановки
        geo::Coordinates coordinates;

        // расстояние до соседней остановки (название остановки / расстояние до неё)
        std::vector<Distance> distances;
    };

    // команда на добавление маршрута в транспортный справочник
    struct AddBusCmd {
        // название маршрута
        std::string name;
        
        // тип маршрута(линейный / кольцевой)
        RouteType route_type = RouteType::LINEAR;
        
        // список названий остановок в маршруте по порядку
        // в кольцевом маршруте первая и последняя остановка совпадают
        std::vector<std::string> stop_names;
    };

    namespace from_char_stream {
        // класс-парсер команд на добавление данных в текстовый справочник из символьного потока
        class DbReader {
        public:
            // парсит команды из символьного потока
            DbReader(std::istream& sin) : sin_(sin) { Parse(); }

            // список команд на добавление остановки
            const std::vector<AddStopCmd>& GetAddStopCmds() const {
                return add_stop_cmds_;
            }

            // cписок команд на добавление маршрута
            const std::vector<AddBusCmd>& GetAddBusCmds() const { return add_bus_cmds_; }

        private:
            void Parse();

            std::istream& sin_;
            std::vector<AddStopCmd> add_stop_cmds_;
            std::vector<AddBusCmd> add_bus_cmds_;
        };

        namespace detail {

            std::vector<std::string_view> SplitNoWS(std::string_view line,
                std::string_view by);
            std::vector<std::string_view> SplitNoWS(std::string_view line, char by);

        }  // namespace detail

        void ReadDB(TransportCatalogue& transport_catalogue, std::istream& sin);

    }  // namespace from_char_stream

}  // namespace transport_catalogue::input_reader
