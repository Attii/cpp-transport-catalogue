#pragma once
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

inline const double EPSILON = 1e-6;
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

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

// int main() {
//     using namespace std;

//     const double WIDTH = 600.0;
//     const double HEIGHT = 400.0;
//     const double PADDING = 50.0;
    
//     // Точки, подлежащие проецированию
//     vector<geo::Coordinates> geo_coords = {
//         {43.587795, 39.716901}, {43.581969, 39.719848}, {43.598701, 39.730623},
//         {43.585586, 39.733879}, {43.590317, 39.746833}
//     };

//     // Создаём проектор сферических координат на карту
//     const SphereProjector proj{
//         geo_coords.begin(), geo_coords.end(), WIDTH, HEIGHT, PADDING
//     };

//     // Проецируем и выводим координаты
//     for (const auto &geo_coord: geo_coords) {
//         const svg::Point screen_coord = proj(geo_coord);
//         cout << '(' << geo_coord.lat << ", "sv << geo_coord.lng << ") -> "sv;
//         cout << '(' << screen_coord.x << ", "sv << screen_coord.y << ')' << endl;
//     }
// }