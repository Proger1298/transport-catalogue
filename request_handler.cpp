#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

svg::Document RequestHandler::RenderMap() const {
    return renderer_.MakeSVGDocument(db_.GetBusesSortedByName());
}

const transport::TransportRouter::CompleteRouteInfo RequestHandler::GetOptimalRoute(
    const std::string_view stop_from_name,
    const std::string_view stop_to_name
) const {
    return transport_router_.FindOptimalRoute(
        db_.FindStop(stop_from_name),
        db_.FindStop(stop_to_name)
    );
}