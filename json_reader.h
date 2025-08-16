#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <sstream>

using namespace std::literals;

class JsonReader {
public:
    explicit JsonReader(std::istream& request) 
    : request_(std::move(json::Load(request)))
    {

    }

    void ApplyBaseRequests(transport::TransportCatalogue& catalogue);

    void ParseStatAndPrepareAnswer(const transport::TransportCatalogue& catalogue, const RequestHandler& request_handler);

    renderer::RenderSettings ParseRenderSettings() const;
    transport::TransportRouteSettings ParseRouteSettings() const;

    void PrintJSON(std::ostream& output);

private:
    json::Document request_;
    json::Array answer_;

    const json::Array& GetBaseRequests() const;
    const json::Array& GetStatRequests() const;
    const json::Dict& GetRenderSettings() const;
    const json::Dict& GetRoutingSettings() const;

    void AddStop(const json::Dict& stop_dict, transport::TransportCatalogue& catalogue);
    void SetDistancesBetweenStops(const json::Dict& stop_dict, transport::TransportCatalogue& catalogue);

    std::vector<std::string_view> ParseBusStops(const json::Array& bus_stops_array);
    void AddBus(const json::Dict& bus_dict, transport::TransportCatalogue& catalogue);

    svg::Color ParseColor(const json::Node& color_node) const;

    const json::Dict PrepareBusAnswer(const transport::TransportCatalogue& catalogue, const json::Dict& cur_dict) const;
    const json::Dict PrepareStopAnswer(const transport::TransportCatalogue& catalogue, const json::Dict& cur_dict) const;
    const json::Dict PrepareMapAnswer(const RequestHandler& request_handler, const json::Dict& cur_dict) const;
    const json::Dict PrepareRouteAnswer(const RequestHandler& request_handler, const json::Dict& cur_dict) const;
};