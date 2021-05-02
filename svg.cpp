#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(const Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    const Point& Circle::getCenter() const {
        return center_;
    }

    double Circle::getRadius() const {
        return radius_;
    }


    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(const Point point) {
        points_.push_back(point);
        return *this;
    }

    const std::vector<Point>& Polyline::getPoints() const {
        return points_;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        int size = points_.size();
        out << "<polyline points=\""sv;
        if (!points_.empty()) {
            for (const auto& point : points_) {
                if (size > 1) {
                    out << point.x << "," << point.y << " ";
                } else {
                    out << point.x << "," << point.y << "\" ";
                }
                --size;
            }
            RenderAttrs(out);
            out << "/>"sv;
        } else {
            out << "\" ";
            RenderAttrs(out);
            out << "/>"sv;
        }
    }

    // ---------- Text ------------------
    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    const Point& Text::getPosition() const {
        return position_;
    }

    const Point& Text::getOffset() const {
        return offset_;
    }

    uint32_t Text::getFontSize() const {
        return font_size_;
    }

    const std::optional<std::string>& Text::getFontFamily() const {
        return font_family_;
    }

    const std::optional<std::string>& Text::getFontWeight() const {
        return font_weight_;
    }

    const std::string& Text::getData() const {
        return data_;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = ReplacingCharacters(std::move(data));
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text ";
        RenderAttrs(out);
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\"";
        if (font_family_) {
            out << " font-family=\""sv << *font_family_ << "\"";
        }
        if (font_weight_) {
            out << " font-weight=\""sv << *font_weight_ << "\"";
        }
        out << ">" << data_ << "</text>"sv;
    }

    std::string Text::ReplacingCharacters(const std::string& data) {
        std::string result;
        for (const auto& letter: data) {
            switch (letter) {
                case '\"':
                    result += "&quot;";
                    break;
                case '\'':
                    result += "&apos;";
                    break;
                case '<':
                    result += "&lt;";
                    break;
                case '>':
                    result += "&gt;";
                    break;
                case '&':
                    result += "&amp;";
                    break;
                default:
                    result += letter;
            }

        }
        return result;
    }

    // ---------- Text ------------------
    void Document::AddPtr(std::unique_ptr<Object> obj) {
        docs.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << '\n';
        out << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)" << '\n';
        for (const auto& doc : docs) {
            doc.get()->Render(RenderContext(out, 0, 2));
        }
        out << "</svg>"sv;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& lineCap) {
        switch (lineCap) {
            case StrokeLineCap::BUTT:
                out << "butt";
                break;
            case StrokeLineCap::ROUND:
                out << "round";
                break;
            case StrokeLineCap::SQUARE:
                out << "square";
                break;
            default:
                out.setstate(std::ios_base::failbit);
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& lineJoin) {
        switch (lineJoin) {
            case StrokeLineJoin::ARCS:
                out << "arcs";
                break;
            case StrokeLineJoin::BEVEL:
                out << "bevel";
                break;
            case StrokeLineJoin::MITER:
                out << "miter";
                break;
            case StrokeLineJoin::MITER_CLIP:
                out << "miter-clip";
                break;
            case StrokeLineJoin::ROUND:
                out << "round";
                break;
            default:
                out.setstate(std::ios_base::failbit);
        }
        return out;
    }

    void OstreamColorOut::operator()(std::monostate) {
        out << "none"sv;
    }

    void OstreamColorOut::operator()(std::string color) {
        out << color;
    }

    void OstreamColorOut::operator()(Rgb color) {
        out << "rgb("s << static_cast<int>(color.red) << ","s << static_cast<int>(color.green) << ","s
            << static_cast<int>(color.blue) << ")"s;
    }

    void OstreamColorOut::operator()(Rgba color) {
        out << "rgba("s << static_cast<int>(color.red) << ","s << static_cast<int>(color.green) << ","s
            << static_cast<int>(color.blue) << ","s << color.opacity << ")"s;
    }

    Rgb::Rgb(uint8_t red, uint8_t green, uint8_t blue) : red(red), green(green), blue(blue) {}

    Rgb::Rgb() {
    }

    Rgba::Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) : red(red), green(green), blue(blue),
                                                                           opacity(opacity) {}

    Rgba::Rgba() {
    }

}  // namespace svg