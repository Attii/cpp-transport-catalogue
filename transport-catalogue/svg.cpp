#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, StrokeLineCap  var) {
    switch (var)
    {
    case StrokeLineCap::BUTT:
        out <<  "butt";
        break;
    
    case  StrokeLineCap::ROUND:
        out  <<  "round";
        break;
    
    default:
        out << "square";
        break;
    }
    
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin  var) {
    switch (var)
    {
    case StrokeLineJoin::ARCS:
        out <<  "arcs";
        break;
    
    case StrokeLineJoin::BEVEL:
        out <<  "bevel";
        break;
    
    case StrokeLineJoin::MITER:
        out <<  "miter";
        break;

    case StrokeLineJoin::MITER_CLIP:
        out <<  "miter-clip";
        break;

    default:
        out << "round";
        break;
    }
    
    return out;
}

std::ostream& operator<<(std::ostream& out, std::monostate) {
    return out << "none";
}

std::ostream& operator<<(std::ostream& out, const Rgb& color) {
    return  out << "rgb(" 
                << unsigned(color.red) << "," 
                << unsigned(color.green) << ','  
                << unsigned(color.blue) << ")";
}

std::ostream& operator<<(std::ostream& out, const Rgba& color) {
    return  out << "rgba(" 
                << unsigned(color.red) << "," 
                << unsigned(color.green) << ','  
                << unsigned(color.blue) << "," 
                << color.opacity <<  ")";
}

std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(
            [&out](auto val) { 
                out << val;
            }, color);
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
    //context.RenderIndent();
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    this->RenderAttrs(out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);

    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    //context.RenderIndent();
    context.out << "<polyline points=\"";
    bool first = true;
    for (const auto& point : points_) {
        if (!first) {
            context.out << " ";
        }
        context.out << point.x << "," << point.y;
        first = false;
    }
    context.out << "\"";
    this->RenderAttrs(context.out);
    context.out << "/>";
}

Text& Text::SetPosition(Point pos) {
    ref_point_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t font_size) {
    font_size_ = font_size;
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
    context.out << "<text";
    this->RenderAttrs(context.out);
    context.out << " x=\"" << ref_point_.x << "\"";
    context.out << " y=\"" << ref_point_.y << "\"";
    context.out << " dx=\"" << offset_.x << "\"";
    context.out << " dy=\"" << offset_.y << "\"";
    context.out << " font-size=\"" << font_size_ << "\"";
    if (!font_family_.empty()) {
        context.out << " font-family=\"" << font_family_ << "\"";
    }
    if (!font_weight_.empty()) {
        context.out << " font-weight=\"" << font_weight_ << "\"";
    }
    context.out << ">" << data_ << "</text>";
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    
    RenderContext ctx{out, 2, 2};
    for(const auto& obj : objects_) {
        obj.get()->Render(ctx);
    }

    out << "</svg>"sv;
}

}  // namespace svg