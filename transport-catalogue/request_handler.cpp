#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

void RequestHandler::ProcessJSON(const json::Node& node) {
    InputReader reader;

    reader.ParseJSON(node);
    reader.ApplyBaseRequests(db_);

    renderer_.SetRenderSettings(reader.GetRenderSettings());
    renderer_.SetSphereProjector(db_);
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.RenderMap(db_);
}