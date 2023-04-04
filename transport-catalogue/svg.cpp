#include "svg.h"

namespace svg {

    using namespace std::literals;
    using namespace std;

    ostream& operator<<(ostream& out, svg::StrokeLineCap line_cap) {
        using svg::StrokeLineCap;

        switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        default:
            throw logic_error("Not implemented"s);
        }
        return out;
    }

    ostream& operator<<(ostream& out, svg::StrokeLineJoin line_join) {
        using svg::StrokeLineJoin;

        switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        default:
            throw logic_error("Not implemented"s);
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Color ------------------

    bool Rgb::operator==(Rgb other) const {
        return red == other.red && green == other.green && blue == other.blue;
    }

    bool Rgba::operator==(Rgba other) const {
        return red == other.red && green == other.green && blue == other.blue &&
            opacity == other.opacity;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv
            << "r=\""sv << radius_ << '"';
        this->PathProps::RenderAttrs(out);
        out << " />"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        this->points_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first = true;
        for (Point p : points_) {
            if (!first) {
                out.put(' ');
            }
            out << p.x << ',' << p.y;
            first = false;
        }
        out.put('"');
        this->PathProps::RenderAttrs(out);
        out << " />"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_.emplace(move(font_weight));
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_.emplace(move(font_family));
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" dx=\""sv
            << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv
            << font_size_ << '"';
        if (font_family_) {
            out << " font-family=\""sv << *font_family_ << '"';
        }
        if (font_weight_) {
            out << " font-weight=\""sv << *font_weight_ << '"';
        }
        this->PathProps::RenderAttrs(out);
        out << '>' << data_ << "</text>"sv;
    }


    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl
            << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext render_ctx{ out, 2, 2 };
        for (const auto& obj : objects_) {
            obj->Render(render_ctx);
        }
        out << "</svg>"sv << std::endl;
    }

}  // namespace svg
