#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "domain.h"
#include "geo.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace request_handler {

    using namespace transport_catalogue;
    using map_renderer::MapRenderer;
    using map_renderer::RenderSettings;
    using transport_catalogue::router::RouterSettings;

    // Команда на добавление остановки в транспортный справочник
    struct AddStopCmd {
        // (название остановки / расстояние до неё в метрах)
        using Distance = std::pair<std::string, size_t>;

        // Имя остановки
        std::string name;

        // Координаты остановки
        geo::Coordinates coordinates;

        // Расстояния до соседних остановок
        std::vector<Distance> distances;
    };

    // Команда на добавление маршрута в транспортный справочник
    struct AddBusCmd {
        // Название маршрута
        std::string name;

        // Тип маршрута( кольцевой/линейный )
        RouteType route_type = RouteType::LINEAR;

        // Список названий остановок в маршруте по порядку
        std::vector<std::string> stop_names;
    };

    // Базовый класс для всех запросов статистики к транспортному справочнику
    struct BaseStatRequest {
        int id = 0;
    };

    // Запрос на статистику по маршруту
    struct BusStatRequest : public BaseStatRequest {
        std::string name;
    };

    // Запрос на статистику по остановке
    struct StopStatRequest : public BaseStatRequest {
        std::string name;
    };

    // Запрос на получение карты маршрутов в SVG формате
    struct MapRequest : public BaseStatRequest {};

    struct RouteRequest : public BaseStatRequest {
        std::string from;
        std::string to;
    };

    // Все возможные типы запросов на наполнеие базы транспортного справочника
    using BaseRequest = std::variant<AddStopCmd, AddBusCmd>;

    // Все возможные типы запросов на получение статистики из транспортного справочника
    using StatRequest =
        std::variant<BusStatRequest, StopStatRequest, MapRequest, RouteRequest>;

    // Базовый класс для получения запросов к транспортному справочнику
    class AbstractBufferingRequestReader {
    public:
        // Запросы на наполнение базы транспортного справочника
        virtual const std::vector<BaseRequest>& GetBaseRequests() const = 0;

        // Запросы на получение статистики из транспортного справочника
        virtual const std::vector<StatRequest>& GetStatRequests() const = 0;

        // Запрос на получение настроек отрисовки карты
        virtual const std::optional<RenderSettings>& GetRenderSettings() const = 0;

        // Запрос на получение настроек отрисовки маршрута
        virtual const std::optional<RouterSettings>& GetRouterSettings() const = 0;

    protected:
        // Не разрешаем полиморфное владение наследниками этого класса
        ~AbstractBufferingRequestReader() = default;
    };

    // Ответ на запрос на получение статистики по маршруту
    struct BusStatResponse {
        BusStats bus_stats;
    };

    // Ответ на запрос на получение статистики по остановке
    struct StopStatResponse {
        BusesForStop buses_for_stop;
    };

    // Ответ на запрос на получение карты в SVG формате
    struct MapResponse {
        std::string svg_map;
    };

    // Все возможные типы ответов на запросы на получение статистики
    using StatResponse =
        std::variant<std::monostate, BusStatResponse, StopStatResponse, MapResponse, router::RouteResult>;

    // Базовый класс для печати ответов на запросы к транспортному справочнику
    class AbstractStatResponsePrinter {
    public:
        virtual void PrintResponse(int request_id, const StatResponse&) = 0;

    protected:
        ~AbstractStatResponsePrinter() = default;
    };

    class BufferingRequestHandler {
    public:
        BufferingRequestHandler(TransportCatalogue& transport_catalogue,
            const AbstractBufferingRequestReader& request_reader)
            : transport_catalogue_(transport_catalogue),
            request_reader_(request_reader) {}
        void ProcessRequests(AbstractStatResponsePrinter& stat_response_printer);
        void RenderMap(MapRenderer&);

    private:
        TransportCatalogue& transport_catalogue_;
        const AbstractBufferingRequestReader& request_reader_;
    };

}  // namespace transport_catalogue::request_handler
