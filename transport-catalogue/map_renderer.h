#pragma once
#include "geo.h"
#include "svg.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

struct Offset {
    double dx;
    double dy;
};

class RenderSettings
{
public:
    // Width
    void SetWidth(double width) {
        width_ = width;
    }
    double GetWidth() const {
        return width_;
    }

    // Height
    void SetHeight(double height) {
        height_ = height;
    }
    double GetHeight() const {
        return height_;
    }

    // Padding
    void SetPadding(double padding_value) {
        padding = padding_value;
    }
    double GetPadding() const {
        return padding;
    }

    // Line Width
    void SetLineWidth(double lw) {
        line_width = lw;
    }
    double GetLineWidth() const {
        return line_width;
    }

    // Stop Radius
    void SetStopRadius(double radius) {
        stop_radius = radius;
    }
    double GetStopRadius() const {
        return stop_radius;
    }

    // Bus Label Font Size
    void SetBusLabelFontSize(int size) {
        bus_label_font_size = size;
    }
    int GetBusLabelFontSize() const {
        return bus_label_font_size;
    }

    // Bus Label Offset
    void SetBusLabelOffset(const Offset& offset) {
        bus_label_offset = offset;
    }
    Offset GetBusLabelOffset() const {
        return bus_label_offset;
    }

    // Stop Label Font Size
    void SetStopLabelFontSize(unsigned size) {
        stop_label_font_size = size;
    }
    unsigned GetStopLabelFontSize() const {
        return stop_label_font_size;
    }

    // Stop Label Offset
    void SetStopLabelOffset(const Offset& offset) {
        stop_label_offset = offset;
    }
    Offset GetStopLabelOffset() const {
        return stop_label_offset;
    }

    // Underlayer Color
    void SetUnderlayerColor(const svg::Color& color) {
        underlayer_color = color;
    }
    svg::Color GetUnderlayerColor() const {
        return underlayer_color;
    }

    // Underlayer Width
    void SetUnderlayerWidth(double width) {
        underlayer_width = width;
    }
    double GetUnderlayerWidth() const {
        return underlayer_width;
    }

    // Color Palette
    void SetColorPalette(const std::vector<svg::Color>& palette) {
        color_palette = palette;
    }
    std::vector<svg::Color> GetColorPalette() const {
        return color_palette;
    }

private:
    double width_, height_;
    double padding;
    double line_width;
    double stop_radius;
    int bus_label_font_size;
    Offset bus_label_offset;
    unsigned stop_label_font_size;
    Offset stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
};

class MapRenderer {
public:
    void SetSphereProjector(const TransportCatalogue& catalogue);
    
    void SetRenderSettings(RenderSettings settings) {settings_ = std::move(settings);};

    svg::Document RenderMap(const TransportCatalogue& catalogue) const;

private:
    // Создание SVG представления маршрутов
    void RenderRouteLines(svg::Document& doc, const TransportCatalogue& catalogue) const;

    // Создание SVG названий маршрутов
    void RenderRouteLabels(svg::Document& doc, const TransportCatalogue& catalogue) const;

    // Создание SVG значков остановок
    void RenderStopSigns(svg::Document& doc, const TransportCatalogue& catalogue) const;

    // Создание SVG названий остановок
    void RenderStopNames(svg::Document& doc, const TransportCatalogue& catalogue) const;

    std::optional<SphereProjector> projector_;
    RenderSettings settings_;
    std::vector<std::string> color_palette_;
    double line_width_;
};

