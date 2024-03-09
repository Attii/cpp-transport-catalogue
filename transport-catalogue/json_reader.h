#pragma once

#include <memory>                                               

#include <sstream>
#include <iostream>

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

using namespace geo;

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

    json::Array ApplyStatRequests(const TransportCatalogue& catalogue, const MapRenderer& renderer) const;

    RenderSettings GetRenderSettings() const {return render_settings_;};

    void RenderMap(TransportCatalogue& catalogue, std::ostream& out) const;

private:
    std::vector<std::unique_ptr<BaseRequestsDescription>> base_requests_;
    RenderSettings render_settings_;
    std::vector<StatRequest> stat_requests_;
};