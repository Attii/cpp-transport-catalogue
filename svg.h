#pragma once
#define _USE_MATH_DEFINES 
#include  <cmath>

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {

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

struct Rgb
{
    Rgb()  = default;
    Rgb(uint8_t r, uint8_t g, uint8_t b) 
        : red(r)
        , green(g)
        , blue(b)  
    {
    }

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba : public Rgb
{
    Rgba()  = default;
    Rgba(uint8_t r, uint8_t g, uint8_t b, double a) 
        : Rgb(r, g, b)
        , opacity(a)
    {
    }
    
    double  opacity = 1.0;
};

using Color = std::variant<std::monostate, Rgb, Rgba, std::string>; 
inline const Color NoneColor{"none"};

std::ostream& operator<<(std::ostream& out, StrokeLineCap  var);
std::ostream& operator<<(std::ostream& out, StrokeLineJoin  var);

std::ostream& operator<<(std::ostream& out, std::monostate);
std::ostream& operator<<(std::ostream& out, const Rgb& color);
std::ostream& operator<<(std::ostream& out, const Rgba& color);
std::ostream& operator<<(std::ostream& out, const Color& color);


template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }

    Owner&  SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = line_cap;
        return AsOwner();
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if (stroke_width_) {
            out <<  " stroke-width=\""sv << *stroke_width_ << "\""sv; 
        }
        if (line_cap_) {
            out  <<  " stroke-linecap=\"" << *line_cap_ << "\""sv;
        }
        if (line_join_)  {
            out  <<  " stroke-linejoin=\"" << *line_join_ <<  "\""sv;  
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
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
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
        return {out, indent_step, indent + indent_step};
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
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

    ~Circle() noexcept override = default;

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
    Polyline& AddPoint(Point point);

    ~Polyline() noexcept override = default;

private:
    void RenderObject(const RenderContext& context) const override;

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

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    ~Text() noexcept override = default;

private:
    void RenderObject(const RenderContext& context) const override;
    
    Point ref_point_;
    Point offset_;
    uint32_t font_size_ = 1;
    std::string font_family_, font_weight_, data_;
};

class ObjectContainer {
public: 
    template <typename Obj>
    void Add(Obj obj) {
        objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

protected:
    std::vector<std::shared_ptr<Object>> objects_;
};

class Document : public ObjectContainer {
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;
};

class Drawable {
public: 
    virtual void Draw(ObjectContainer& container) const = 0; 

    virtual  ~Drawable() = default;
};

}  // namespace svg


namespace shapes {

class Triangle : public svg::Drawable {
public:
    Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
        : p1_(p1)
        , p2_(p2)
        , p3_(p3) {
    }

    // Реализует метод Draw интерфейса svg::Drawable
    void Draw(svg::ObjectContainer& container) const override {
        container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
    }

private:
    svg::Point p1_, p2_, p3_;
};

class Star : public svg::Drawable {
public:
    Star(svg::Point center, double outer_radius, double inner_radius, int num_rays) 
        : center_(center)
        , outer_radius_(outer_radius)
        , inner_radius_(inner_radius)
        , num_rays_(num_rays)
    {
    }

    void Draw(svg::ObjectContainer& container) const override  {
        container.Add(CreateStar(center_,  outer_radius_, inner_radius_, num_rays_).SetFillColor(std::string("red")).SetStrokeColor("black"));
    }

private:
    svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) const {
        using namespace svg;
        Polyline polyline;
        for (int i = 0; i <= num_rays; ++i) {
            double angle = 2 * M_PI * (i % num_rays) / num_rays;
            polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
            if (i == num_rays) {
                break;
            }
            angle += M_PI / num_rays;
            polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
        }
        return polyline;
    }   

    svg::Point center_;
    double outer_radius_;
    double inner_radius_;
    int num_rays_;
};

class Snowman : public svg::Drawable {
public:
    Snowman(svg::Point head_center,  double head_rad) 
        :  head_center_(head_center)
        ,  head_radius_(head_rad)
    {
    }

    void Draw(svg::ObjectContainer& container) const override  {
        svg::Point tmp = {head_center_.x, head_center_.y + 5 * head_radius_};
        container.Add(svg::Circle().SetCenter(tmp).SetRadius(head_radius_ + head_radius_).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));

        tmp = {head_center_.x, head_center_.y + 2 * head_radius_};
        container.Add(svg::Circle().SetCenter(tmp).SetRadius(head_radius_ + 0.5 * head_radius_).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
        
        container.Add(svg::Circle().SetCenter(head_center_).SetRadius(head_radius_).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
    }

private:
    svg::Point head_center_;
    double head_radius_;
};
}