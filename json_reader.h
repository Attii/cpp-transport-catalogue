#pragma once

#include <memory>                                               

#include <iostream>

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

using namespace geo;

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
    std::optional<double> GetWidth() const {
        return width_;
    }

    // Height
    void SetHeight(double height) {
        height_ = height;
    }
    std::optional<double> GetHeight() const {
        return height_;
    }

    // Padding
    void SetPadding(double padding_value) {
        padding = padding_value;
    }
    std::optional<double> GetPadding() const {
        return padding;
    }

    // Line Width
    void SetLineWidth(double lw) {
        line_width = lw;
    }
    std::optional<double> GetLineWidth() const {
        return line_width;
    }

    // Stop Radius
    void SetStopRadius(double radius) {
        stop_radius = radius;
    }
    std::optional<double> GetStopRadius() const {
        return stop_radius;
    }

    // Bus Label Font Size
    void SetBusLabelFontSize(int size) {
        bus_label_font_size = size;
    }
    std::optional<int> GetBusLabelFontSize() const {
        return bus_label_font_size;
    }

    // Bus Label Offset
    void SetBusLabelOffset(const Offset& offset) {
        bus_label_offset = offset;
    }
    std::optional<Offset> GetBusLabelOffset() const {
        return bus_label_offset;
    }

    // Stop Label Font Size
    void SetStopLabelFontSize(unsigned size) {
        stop_label_font_size = size;
    }
    std::optional<unsigned> GetStopLabelFontSize() const {
        return stop_label_font_size;
    }

    // Stop Label Offset
    void SetStopLabelOffset(const Offset& offset) {
        stop_label_offset = offset;
    }
    std::optional<Offset> GetStopLabelOffset() const {
        return stop_label_offset;
    }

    // Underlayer Color
    void SetUnderlayerColor(const svg::Color& color) {
        underlayer_color = color;
    }
    std::optional<svg::Color> GetUnderlayerColor() const {
        return underlayer_color;
    }

    // Underlayer Width
    void SetUnderlayerWidth(double width) {
        underlayer_width = width;
    }
    std::optional<double> GetUnderlayerWidth() const {
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
    std::optional<double> width_, height_;
    std::optional<double> padding;
    std::optional<double> line_width;
    std::optional<double> stop_radius;
    std::optional<int> bus_label_font_size;
    std::optional<Offset> bus_label_offset;
    std::optional<unsigned> stop_label_font_size;
    std::optional<Offset> stop_label_offset;
    std::optional<svg::Color> underlayer_color;
    std::optional<double> underlayer_width;
    std::vector<svg::Color> color_palette;
};


class InputReader {
public:

    /**
     * Парсит узел JSON в структуру BaseRequestsDescription и сохраняет результат в base_requests_
     */
    void ParseJSON(const json::Node& node);

    /**
     * Наполняет данными транспортный справочник, используя команды из base_requests_
     */
    void ApplyBaseRequests(TransportCatalogue& catalogue) const;

    json::Array ApplyStatRequests(TransportCatalogue& catalogue) const;

private:
    std::vector<std::unique_ptr<BaseRequestsDescription>> base_requests_;
    RenderSettings render_settings;
    std::vector<StatRequest> stat_requests_;
};