#pragma once

#include "request_handler.h"
#include "map_renderer.h"

namespace json_reader {

    using map_renderer::RenderSettings;

    class BufferingRequestReader final : public request_handler::AbstractBufferingRequestReader {
    public:
        // Парсит команды из потока
        BufferingRequestReader(std::istream& in) { Parse(in); }

        virtual const std::vector<request_handler::BaseRequest>& GetBaseRequests() const override {
            return base_requests_;
        }

        virtual const std::vector<request_handler::StatRequest>& GetStatRequests() const override {
            return stat_requests_;
        }

        // Возвращает настройки отрисовки карты в SVG формате
        virtual const std::optional<RenderSettings>& GetRenderSettings() const override {
            return render_settings_;
        }

    private:
        std::vector<request_handler::BaseRequest> base_requests_;
        std::vector<request_handler::StatRequest> stat_requests_;
        std::optional<RenderSettings> render_settings_;

        void Parse(std::istream&);
    };

    class ResponsePrinter final : public request_handler::AbstractStatResponsePrinter {
    public:
        ResponsePrinter(std::ostream&);
        virtual void PrintResponse(int request_id, const request_handler::StatResponse&) override;
        ~ResponsePrinter();

    private:
        std::ostream& out_;

        bool printed_something_ = false;

        void Begin();
        void End();
    };
}
