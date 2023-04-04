#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <string_view>
#include <utility>
#include <deque>

namespace svg {
    struct Rgb {
        Rgb() = default;
        Rgb(unsigned int _red, unsigned int _green, unsigned int _blue)
            : red(_red), green(_green), blue(_blue) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;

        bool operator==(Rgb other) const;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(unsigned int _red, unsigned int _green, unsigned int _blue,
            double _opacity)
            : red(_red), green(_green), blue(_blue), opacity(_opacity) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;

        bool operator==(Rgba other) const;
    };

    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

    inline const Color NoneColor{ "none" };

    // Структура для вывода Color (тип Variant) через std::visit()
    struct ColorPrinter
    {
        std::ostream& out;

        void operator()(std::monostate) const {
            out << std::get<std::string>(NoneColor);
        }

        void operator()(std::string str) const
        {
            out << str;
        }

        void operator()(Rgb rgb) const
        {
            using namespace std::literals;
            out << "rgb("s << std::to_string(rgb.red) << ","s << std::to_string(rgb.green) << ","s << std::to_string(rgb.blue) << ")"s;
        }

        void operator()(Rgba rgba) const
        {
            using namespace std::literals;
            out << "rgba("s << std::to_string(rgba.red) << ","s << std::to_string(rgba.green) << ","s << std::to_string(rgba.blue) << ","s << rgba.opacity << ")"s;
        }
    };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    std::ostream& operator<<(std::ostream& out, svg::StrokeLineCap);
    std::ostream& operator<<(std::ostream& out, svg::StrokeLineJoin);

    // Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    /*
     * Вспомогательный базовый класс.
     * Путь — представленный в виде последовательности различных контуров векторный объект,
     * который будет содержать свойства, управляющие параметрами заливки и контура.
     * Унаследовав от него классы Circle, Polyline и Text, мы сделаем их обладателями этих свойств.
     */
    template <typename Owner>
    class PathProps {
    public:
        // Цвет заполнения (атрибут fill)
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        // Цвет контура (атрибут stroke)
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        // Толщина линии контура (атрибут stroke-width)
        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }
        // Тип формы конца линии (атрибут stroke-linecap)
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = line_cap;
            return AsOwner();
        }
        // Тип формы соединения линий (атрибут stroke-linejoin)
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        // Выводит выставленные свойства заливки и контура в поток
        void RenderAttrs(std::ostream& out) const
        {
            using namespace std::literals;

            if (fill_color_)
            {
                out << " fill=\""sv;
                std::visit(ColorPrinter{ out }, *fill_color_);
                out << "\""sv;
            }
            if (stroke_color_)
            {
                out << " stroke=\""sv;
                std::visit(ColorPrinter{ out }, *stroke_color_);
                out << "\""sv;
            }
            if (stroke_width_)
            {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_line_cap_)
            {
                out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
            }
            if (stroke_line_join_)
            {
                out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
            }
        }


    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

    private:
        virtual void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

    private:
        virtual void RenderObject(const RenderContext& context) const override;

        Point pos_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::optional<std::string> font_family_;
        std::optional<std::string> font_weight_;
        std::string data_;
    };

    class ObjectContainer {
    public:
        // Метод Add добавляет в контейнер любой объект-наследник svg::Object
        template <typename T>
        void Add(T obj) {
            AddPtr(std::make_unique<T>(std::move(obj)));
        }

        // Добавляет в контейнер объект-наследник svg::Object
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    protected:
        ~ObjectContainer() = default;
    };

    class Document : public ObjectContainer {
    public:
        // Метод Add добавляет в svg - документ любой объект - наследник svg::Object

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Drawable {
    public:
        virtual ~Drawable() = default;
        virtual void Draw(ObjectContainer& obj_container) const = 0;
    };

}  // namespace svg
