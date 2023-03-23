#pragma once

#include <istream>
#include <optional>
#include <string_view>

#include "transport_catalogue.h"

namespace transport_catalogue::stat_reader {

    namespace to_char_stream {

       // класс для печати статистики о маршрутах и остановках в символьный поток
        class StatsPrinter {
        public:
            StatsPrinter(std::ostream& sout) : sout_(sout) {}

            void PrintBusStats(std::string_view bus_name,
                const std::optional<BusStats>& bus_stats);
            void PrintStopInfo(std::string_view stop_name,
                const std::optional<BusesForStop>& stop_info);

        private:
            std::ostream& sout_;
        };

    }  // namespace to_char_stream

    namespace from_char_stream {

        // класс-обработчик запросов на получение статистики из транспортного справочника
        class StatsRequestProcessor {
        public:
            StatsRequestProcessor(std::istream& sin) : sin_(sin) {}

            void ProcessRequests(const TransportCatalogue&,
                to_char_stream::StatsPrinter&);

        private:
            std::istream& sin_;
        };

    }  // namespace from_char_stream

}  // namespace transport_catalogue::stat_reader
