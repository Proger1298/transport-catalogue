#include "svg.h"

namespace svg {

using namespace std::literals;

// ---------- Color ------------------

void СolorPrinter::operator()(std::monostate) const {
    out << "none"sv;
}

void СolorPrinter::operator()(std::string color) const {
    out << color;
}

void СolorPrinter::operator()(const Rgb& rgb_color) const {
    out << "rgb("sv 
    << static_cast<int>(rgb_color.red) << ","sv
    << static_cast<int>(rgb_color.green) << ","sv
    << static_cast<int>(rgb_color.blue)
    << ")"sv;
}

void СolorPrinter::operator()(const Rgba& rgba_color) const {
    out << "rgba("sv 
    << static_cast<int>(rgba_color.red) << ","sv
    << static_cast<int>(rgba_color.green) << ","sv
    << static_cast<int>(rgba_color.blue) << ","sv
    << rgba_color.opacity
    << ")"sv;
}

// Перегруженный оператор вывода цвета в поток с использованием std::visit
std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(СolorPrinter{out}, color);
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& stroke_linecap) {
    switch (stroke_linecap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& stroke_linejoin) {
    switch (stroke_linejoin) {
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
    }
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ----------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(std::move(point));
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool first = true;
    for (const auto& point : points_) {
        if (first) {
            out << point.x << ","sv << point.y;
            first = false;
        } else {
            out << " "sv << point.x << ","sv << point.y;
        }
    }
     out << "\"";
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text ----------------

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

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << " x=\"" << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << font_size_ << "\" "sv;
    if (!font_family_.empty()) {
        out << "font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">"sv << data_ << "</text>"sv;
}

// ---------- Document ----------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    RenderContext ctx(out, 2, 2);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const auto& object : objects_) {
        object->Render(ctx);
    }
    out << "</svg>"sv;
}

}  // namespace svg