#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */


bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Document MapRenderer::RenderMap(const TransportCatalogue& catalogue) const {
    svg::Document doc;
    RenderRouteLines(doc, catalogue);
    RenderRouteLabels(doc, catalogue);
    RenderStopSigns(doc, catalogue);
    RenderStopNames(doc, catalogue);
    return doc;
}

std::vector<geo::Coordinates> GetStopsCoordinates(const TransportCatalogue& catalogue) {
    std::vector<geo::Coordinates> result;

    for(const auto& stop : catalogue.GetStops()) {
        auto stop_info = catalogue.GetStopInfo(stop); 
        if (!stop_info.buses_->empty()) {
            result.push_back(stop_info.stop_.coordinates_);
        }
    }

    return result;
}

void MapRenderer::SetSphereProjector(const TransportCatalogue& catalogue) {
    auto coordinates =  GetStopsCoordinates(catalogue);

    projector_.emplace(coordinates.begin(), coordinates.end(),
                       settings_.GetWidth(), settings_.GetHeight(), 
                       settings_.GetPadding());
}

void MapRenderer::RenderRouteLines(svg::Document& doc, const TransportCatalogue& catalogue) const {
    const auto& bus_to_stops = catalogue.GetRoutes();
    auto routes = catalogue.GetRoutesName();

    int color_index = 0;
    for(const auto& route : routes) {
        const auto& stops = bus_to_stops.at(route); 
        if (stops.empty()) {
            continue;
        }

        svg::Polyline route_line;
        route_line.SetStrokeColor(settings_.GetColorPalette()[color_index])
                    .SetFillColor("none")
                    .SetStrokeWidth(settings_.GetLineWidth())
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        color_index = (color_index + 1) % settings_.GetColorPalette().size();

        for (const auto stop :stops) {
            route_line.AddPoint(projector_.value()(stop->coordinates_));
        }

        doc.Add(route_line);
    }
}

void MapRenderer::RenderRouteLabels(svg::Document& doc, const TransportCatalogue& catalogue) const {
    const auto& bus_to_stops = catalogue.GetRoutes();
    auto routes = catalogue.GetRoutesName();

    int color_index = 0;
    for(const auto& route : routes) {
        const auto& stops = bus_to_stops.at(route); 
        if (stops.empty()) {
            continue;
        }
        
        std::vector<const Stop*> route_stops = {stops.front()};
        if(!catalogue.IsRoundTrip(route) && stops.front() != stops[stops.size() / 2]) {
            route_stops.push_back(stops[stops.size() / 2]);
        }
        
        for (const auto stop : route_stops) {
            svg::Point point = projector_.value()(stop->coordinates_);

            svg::Text underlay;
            underlay.SetPosition(point)
                    .SetOffset(svg::Point(settings_.GetBusLabelOffset().dx, settings_.GetBusLabelOffset().dy))
                    .SetFontSize(settings_.GetBusLabelFontSize())
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetData(std::string(route));



            svg::Text label = underlay;
            label.SetFillColor(settings_.GetColorPalette()[color_index]);

            underlay.SetStrokeWidth(settings_.GetUnderlayerWidth())
                    .SetFillColor(settings_.GetUnderlayerColor())
                    .SetStrokeColor(settings_.GetUnderlayerColor())
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            doc.Add(underlay);
            doc.Add(label);
        }
        
        color_index = (color_index + 1) % settings_.GetColorPalette().size();
    }
}

void MapRenderer::RenderStopSigns(svg::Document& doc, const TransportCatalogue& catalogue) const {
    for(const auto& stop : catalogue.GetStopsName()) {
        const auto& stop_info = catalogue.GetStopInfo(stop);
        if (stop_info.buses_->empty()) {
            continue;
        }

        svg::Circle stop_sign;

        stop_sign.SetCenter(projector_.value()(stop_info.stop_.coordinates_))
                 .SetRadius(settings_.GetStopRadius())
                 .SetFillColor("white");

        doc.Add(stop_sign);
    }
}

void MapRenderer::RenderStopNames(svg::Document& doc, const TransportCatalogue& catalogue) const {
    for(const auto& stop : catalogue.GetStopsName()) {
        const auto& stop_info = catalogue.GetStopInfo(stop);
        if (stop_info.buses_->empty()) {
            continue;
        }

        svg::Text underlay;
        underlay.SetPosition(projector_.value()(stop_info.stop_.coordinates_))
                .SetOffset(svg::Point(settings_.GetStopLabelOffset().dx, settings_.GetStopLabelOffset().dy))
                .SetFontSize(settings_.GetStopLabelFontSize())
                .SetFontFamily("Verdana")
                .SetData(std::string(stop_info.stop_.name_));

        svg::Text stop_name = underlay;
        stop_name.SetFillColor("black");

        underlay.SetFillColor(settings_.GetUnderlayerColor())
                .SetStrokeColor(settings_.GetUnderlayerColor())
                .SetStrokeWidth(settings_.GetUnderlayerWidth())
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);


        doc.Add(underlay);
        doc.Add(stop_name);
    }
}
