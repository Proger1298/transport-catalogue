#include "json_builder.h"
#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

const json::Array& JsonReader::GetBaseRequests() const {
    return request_.GetRoot().AsDict().at("base_requests"s).AsArray();
}

const json::Array& JsonReader::GetStatRequests() const {
    return request_.GetRoot().AsDict().at("stat_requests"s).AsArray();
}

const json::Dict& JsonReader::GetRenderSettings() const {
    return request_.GetRoot().AsDict().at("render_settings"s).AsDict();
}

const json::Dict& JsonReader::GetRoutingSettings() const {
    return request_.GetRoot().AsDict().at("routing_settings"s).AsDict();
}

void JsonReader::AddStop(const json::Dict& stop_dict, transport::TransportCatalogue& catalogue) {
    catalogue.AddStop({
        stop_dict.at("name"s).AsString(),
        {stop_dict.at("latitude"s).AsDouble(), stop_dict.at("longitude"s).AsDouble()}
    });
}

void JsonReader::SetDistancesBetweenStops(const json::Dict& stop_dict, transport::TransportCatalogue& catalogue) {
    const transport::Stop* base_stop = catalogue.FindStop(stop_dict.at("name"s).AsString());
    const json::Dict& names_and_distances_dict = stop_dict.at("road_distances"s).AsDict();
    for (const auto& [stop_name, distance] : names_and_distances_dict) {
        catalogue.SetDistanceBetweenStops(
            base_stop,
            catalogue.FindStop(stop_name),
            distance.AsInt()
        );
    }
}

std::vector<std::string_view> JsonReader::ParseBusStops(const json::Array& bus_stops_array) {
    std::vector<std::string_view> bus_stop_names;
    for (const json::Node& stop_as_node : bus_stops_array) {
        bus_stop_names.push_back(stop_as_node.AsString());
    }
    return bus_stop_names;
}

void JsonReader::AddBus(const json::Dict& bus_dict, transport::TransportCatalogue& catalogue) {
    std::vector<const transport::Stop*> bus_stops;
    const json::Array& stops_from_json = bus_dict.at("stops"s).AsArray();
    for (const std::string_view& stop_name : ParseBusStops(stops_from_json)) {
        bus_stops.push_back(catalogue.FindStop(stop_name));
    }
    catalogue.AddBus({bus_dict.at("name"s).AsString(), bus_stops, bus_dict.at("is_roundtrip"s).AsBool()});
}

void JsonReader::ApplyBaseRequests(transport::TransportCatalogue& catalogue) {
    const json::Array& base_requests = GetBaseRequests();
    for (const json::Node& request : base_requests) {
        const json::Dict& cur_dict = request.AsDict();
        if (cur_dict.at("type"s).AsString() == "Stop"s) {
            AddStop(cur_dict, catalogue);
        }
    }
    for (const json::Node& request : base_requests) {
        const json::Dict& cur_dict = request.AsDict();
        if (cur_dict.at("type"s).AsString() == "Stop"s) {
            SetDistancesBetweenStops(cur_dict, catalogue);
        }
    }
    for (const json::Node& request : base_requests) {
        const json::Dict& cur_dict = request.AsDict();
        if (cur_dict.at("type"s).AsString() == "Bus"s) {
            AddBus(cur_dict, catalogue);
        }
    }
}

const json::Dict JsonReader::PrepareBusAnswer(const transport::TransportCatalogue& catalogue, const json::Dict& cur_dict) const {
    json::Dict stat;
    try {
        const transport::BusInfo result = catalogue.GetBusInfo(catalogue.FindBus(cur_dict.at("name"s).AsString()));
        stat = json::Builder{}
                .StartDict()
                    .Key("curvature"s).Value(result.curvature)
                    .Key("request_id"s).Value(cur_dict.at("id").AsInt())
                    .Key("route_length"s).Value(result.route_length)
                    .Key("stop_count"s).Value(static_cast<int>(result.stop_count))
                    .Key("unique_stop_count"s).Value(static_cast<int>(result.unique_stop_count))
                .EndDict()
            .Build()
        .AsDict();
    } catch (const std::invalid_argument& e) {
        stat = json::Builder{}
                .StartDict()
                    .Key("request_id"s).Value(cur_dict.at("id").AsInt())
                    .Key("error_message"s).Value("not found"s)
                .EndDict()
            .Build()
        .AsDict();
    }
    return stat;
}

const json::Dict JsonReader::PrepareStopAnswer(const transport::TransportCatalogue& catalogue, const json::Dict& cur_dict) const {
    const transport::Stop* stop = catalogue.FindStop(cur_dict.at("name"s).AsString());
    json::Dict stat;
    if (!stop) {
        stat = json::Builder{}
                .StartDict()
                    .Key("request_id"s).Value(cur_dict.at("id").AsInt())
                    .Key("error_message"s).Value("not found"s)
                .EndDict()
            .Build()
        .AsDict();
    } else {
        json::Array buses;
        for (const transport::Bus* bus : catalogue.GetStopToBuses(stop)) {
            buses.push_back(bus->bus_name);
        }
        stat = json::Builder{}
                .StartDict()
                    .Key("request_id"s).Value(cur_dict.at("id").AsInt())
                    .Key("buses"s).Value(buses)
                .EndDict()
            .Build()
        .AsDict();
    }
    return stat;
}

const json::Dict JsonReader::PrepareMapAnswer(const RequestHandler& request_handler, const json::Dict& cur_dict) const {
    json::Dict stat;
    std::ostringstream stream;
    svg::Document map = request_handler.RenderMap();
    map.Render(stream);
    stat = json::Builder{}
            .StartDict()
                .Key("request_id"s).Value(cur_dict.at("id").AsInt())
                .Key("map"s).Value(stream.str())
            .EndDict()
        .Build()
    .AsDict();
    return stat;
}

const json::Dict JsonReader::PrepareRouteAnswer(const RequestHandler& request_handler, const json::Dict& cur_dict) const {
    json::Dict stat;

    const std::string_view stop_from_name = cur_dict.at("from"s).AsString();
    const std::string_view stop_to_name = cur_dict.at("to"s).AsString();
    
    const transport::TransportRouter::CompleteRouteInfo route_info = request_handler.GetOptimalRoute(
        stop_from_name,
        stop_to_name
    );
    if (!route_info.has_value()) {
        stat = json::Builder{}
                .StartDict()
                    .Key("request_id"s).Value(cur_dict.at("id").AsInt())
                    .Key("error_message"s).Value("not found"s)
                .EndDict()
            .Build()
        .AsDict();
    } else {
        json::Array items;
        items.reserve(route_info.value().first.size());
        for (const transport::TransportRouter::EdgeInfo& edge_info : route_info.value().first) {
            if (std::holds_alternative<transport::TransportRouter::WaitEdgeInfo>(edge_info)) {
                const transport::TransportRouter::WaitEdgeInfo wait_info = std::get<
                    transport::TransportRouter::WaitEdgeInfo
                >(edge_info);
                items.emplace_back(
                    json::Builder{}
                    .StartDict()
                        .Key("type").Value("Wait")
                        .Key("stop_name").Value(wait_info.stop->stop_name)
                        .Key("time").Value(wait_info.bus_wait_time)
                    .EndDict()
                .Build()
                );
            } else {
                const transport::TransportRouter::BusEdgeInfo bus_info = std::get<
                    transport::TransportRouter::BusEdgeInfo
                >(edge_info);
                items.emplace_back(
                    json::Builder{}
                    .StartDict()
                        .Key("type").Value("Bus")
                        .Key("bus").Value(bus_info.bus->bus_name)
                        .Key("span_count").Value(static_cast<int>(bus_info.span_count))
                        .Key("time").Value(bus_info.time)
                    .EndDict()
                .Build()
                );
            }
        }
        stat = json::Builder{}
                .StartDict()
                    .Key("request_id"s).Value(cur_dict.at("id").AsInt())
                    .Key("total_time"s).Value(route_info.value().second)
                    .Key("items"s).Value(items)
                .EndDict()
            .Build()
        .AsDict();
    }

    return stat;
}

void JsonReader::ParseStatAndPrepareAnswer(const transport::TransportCatalogue& catalogue, const RequestHandler& request_handler) {
    const json::Array& stat_requests = GetStatRequests();
    for (const json::Node& request : stat_requests) {
        const json::Dict& cur_dict = request.AsDict();
        json::Dict cur_stat;
        if (cur_dict.at("type"s).AsString() == "Bus"s) {
            cur_stat = PrepareBusAnswer(catalogue, cur_dict);
        } else if (cur_dict.at("type"s).AsString() == "Stop") {
            cur_stat = PrepareStopAnswer(catalogue, cur_dict);
        } else if (cur_dict.at("type"s).AsString() == "Map") {
            cur_stat = PrepareMapAnswer(request_handler, cur_dict);
        } else if (cur_dict.at("type"s).AsString() == "Route") {
            cur_stat = PrepareRouteAnswer(request_handler, cur_dict);
        }
        answer_.push_back(cur_stat);
    }
}

svg::Color JsonReader::ParseColor(const json::Node& color_node) const {
    if (color_node.IsString()) {
        return color_node.AsString();
    } else if (color_node.IsArray()) {
        const json::Array& color_array = color_node.AsArray();
        if (color_array.size() == 3) {
            return svg::Rgb{
                static_cast<uint8_t>(color_array[0].AsInt()),
                static_cast<uint8_t>(color_array[1].AsInt()),
                static_cast<uint8_t>(color_array[2].AsInt())
            };
        } else {
            return svg::Rgba{
                static_cast<uint8_t>(color_array[0].AsInt()),
                static_cast<uint8_t>(color_array[1].AsInt()),
                static_cast<uint8_t>(color_array[2].AsInt()),
                color_array[3].AsDouble()
            };
        }
    }
    return svg::NoneColor;
}

renderer::RenderSettings JsonReader::ParseRenderSettings() const {
    const json::Dict& render_settings_dict = GetRenderSettings();
    renderer::RenderSettings render_settings {
        render_settings_dict.at("width").AsDouble(),
        render_settings_dict.at("height").AsDouble(),
        render_settings_dict.at("padding").AsDouble(),
        render_settings_dict.at("line_width").AsDouble(),
        render_settings_dict.at("stop_radius").AsDouble(),
        render_settings_dict.at("bus_label_font_size").AsInt(),
        {
            render_settings_dict.at("bus_label_offset").AsArray()[0].AsDouble(),
            render_settings_dict.at("bus_label_offset").AsArray()[1].AsDouble()
        },
        render_settings_dict.at("stop_label_font_size").AsInt(),
        {
            render_settings_dict.at("stop_label_offset").AsArray()[0].AsDouble(),
            render_settings_dict.at("stop_label_offset").AsArray()[1].AsDouble()
        },
        ParseColor(render_settings_dict.at("underlayer_color")),
        render_settings_dict.at("underlayer_width").AsDouble(),
        {}
    };
    for (const json::Node& color_node : render_settings_dict.at("color_palette").AsArray()) {
        render_settings.color_palette.push_back(ParseColor(color_node));
    }
    return render_settings;
}

transport::TransportRouteSettings JsonReader::ParseRouteSettings() const {
    const json::Dict& route_settings_dict = GetRoutingSettings();
    transport::TransportRouteSettings route_settings {
        route_settings_dict.at("bus_wait_time").AsInt(),
        route_settings_dict.at("bus_velocity").AsDouble(),
    };
    return route_settings;
}

void JsonReader::PrintJSON(std::ostream& output) {
    json::Print(json::Document{answer_}, output);
}