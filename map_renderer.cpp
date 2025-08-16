#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

std::vector<geo::Coordinates> MapRenderer::GetAllBusesStopsCoordinates(const std::set<const transport::Bus*, transport::BusComparator>& sorted_buses) const {
    std::vector<geo::Coordinates> all_buses_stops_coordiantes;
    for (const transport::Bus* bus : sorted_buses) {
        for (const transport::Stop* stop : bus->stops) {
            all_buses_stops_coordiantes.push_back(stop->coordinates);
        }
    }
    return all_buses_stops_coordiantes;
}

SphereProjector MapRenderer::MakeSphereProjector(const std::vector<geo::Coordinates>& all_buses_stops_coordiantes) const {
    return SphereProjector{
        all_buses_stops_coordiantes.begin(),
        all_buses_stops_coordiantes.end(),
        render_settings_.width,
        render_settings_.height,
        render_settings_.padding
    };
}

std::set<const transport::Stop*, transport::StopComparator> MapRenderer::GetSortedStops(
    const std::set<const transport::Bus*, transport::BusComparator>& sorted_buses
) const {
    std::set<const transport::Stop*, transport::StopComparator> sorted_stops;
    for (const transport::Bus* bus : sorted_buses) {
        for (const transport::Stop* stop : bus->stops) {
            sorted_stops.insert(stop);
        }
    }
    return sorted_stops;
}

std::vector<svg::Polyline> MapRenderer::GenerateBusesLines(
    const std::set<const transport::Bus*, transport::BusComparator>& sorted_buses, const SphereProjector& sphere_projector
) const {
    std::vector<svg::Polyline> buses_lines;
    size_t color_index = 0;
    size_t color_palette_size = render_settings_.color_palette.size();
    for (const transport::Bus* bus : sorted_buses) {
        svg::Polyline cur_bus_line;
        cur_bus_line.
            SetStrokeColor(render_settings_.color_palette.at(color_index % color_palette_size)).
            SetFillColor(svg::NoneColor).
            SetStrokeWidth(render_settings_.line_width).
            SetStrokeLineCap(svg::StrokeLineCap::ROUND).
            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        std::vector<const transport::Stop*> bus_stops{bus->stops.begin(), bus->stops.end()};
        if (!bus->is_roundtrip) {
            bus_stops.insert(bus_stops.end(), std::next(bus->stops.rbegin()), bus->stops.rend());
        }
        for (const transport::Stop* stop : bus_stops) {
            cur_bus_line.AddPoint(sphere_projector(stop->coordinates));
        }
        buses_lines.push_back(cur_bus_line);
        ++color_index;
    }
    return buses_lines;
}

std::vector<svg::Text> MapRenderer::GenerateBusesNames(
    const std::set<const transport::Bus*, transport::BusComparator>& sorted_buses, const SphereProjector& sphere_projector
) const {
    std::vector<svg::Text> buses_name_and_underlayer;
    size_t color_index = 0;
    size_t color_palette_size = render_settings_.color_palette.size();
    for (const transport::Bus* bus : sorted_buses) {
        svg::Text cur_bus_name;
        svg::Text cur_bus_underlayer;
        cur_bus_name.
            SetPosition(sphere_projector(bus->stops.front()->coordinates)).
            SetOffset(render_settings_.bus_label_offset).
            SetFontSize(render_settings_.bus_label_font_size).
            SetFontFamily("Verdana"s).
            SetFontWeight("bold"s).
            SetData(bus->bus_name).
            SetFillColor(render_settings_.color_palette.at(color_index % color_palette_size));

        cur_bus_underlayer.
            SetPosition(sphere_projector(bus->stops.front()->coordinates)).
            SetOffset(render_settings_.bus_label_offset).
            SetFontSize(render_settings_.bus_label_font_size).
            SetFontFamily("Verdana"s).
            SetFontWeight("bold"s).
            SetData(bus->bus_name).
            SetFillColor(render_settings_.underlayer_color).
            SetStrokeColor(render_settings_.underlayer_color).
            SetStrokeWidth(render_settings_.underlayer_width).
            SetStrokeLineCap(svg::StrokeLineCap::ROUND).
            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        buses_name_and_underlayer.push_back(cur_bus_underlayer);
        buses_name_and_underlayer.push_back(cur_bus_name);

        if (!bus->is_roundtrip && bus->stops.front() != bus->stops.back()) {
            svg::Text name_in_last_stop {cur_bus_name};
            svg::Text underlayer_in_last_stop {cur_bus_underlayer};

            name_in_last_stop.SetPosition(sphere_projector(bus->stops.back()->coordinates));
            underlayer_in_last_stop.SetPosition(sphere_projector(bus->stops.back()->coordinates));

            buses_name_and_underlayer.push_back(underlayer_in_last_stop);
            buses_name_and_underlayer.push_back(name_in_last_stop);
        }
        ++color_index;
    }
    return buses_name_and_underlayer;
}

std::vector<svg::Circle> MapRenderer::GenerateStopsCircles(
    const std::set<const transport::Stop*, transport::StopComparator>& sorted_stops, const SphereProjector& sphere_projector
) const {
    std::vector<svg::Circle> stops_circles;
    for (const transport::Stop* stop : sorted_stops) {
        svg::Circle stop_circle;
        stop_circle.
            SetCenter(sphere_projector(stop->coordinates)).
            SetRadius(render_settings_.stop_radius).
            SetFillColor("white"s);
        stops_circles.push_back(stop_circle);
    }
    return stops_circles;
}

std::vector<svg::Text> MapRenderer::GenerateStopsNames(
    const std::set<const transport::Stop*, transport::StopComparator>& sorted_stops, const SphereProjector& sphere_projector
) const {
    std::vector<svg::Text> stops_name_and_underlayer;
    for (const transport::Stop* stop : sorted_stops) {
        svg::Text cur_stop_name;
        svg::Text cur_stop_underlayer;
        cur_stop_name.
            SetPosition(sphere_projector(stop->coordinates)).
            SetOffset(render_settings_.stop_label_offset).
            SetFontSize(render_settings_.stop_label_font_size).
            SetFontFamily("Verdana"s).
            SetData(stop->stop_name).
            SetFillColor("black"s);

        cur_stop_underlayer.
            SetPosition(sphere_projector(stop->coordinates)).
            SetOffset(render_settings_.stop_label_offset).
            SetFontSize(render_settings_.stop_label_font_size).
            SetFontFamily("Verdana"s).
            SetData(stop->stop_name).
            SetFillColor(render_settings_.underlayer_color).
            SetStrokeColor(render_settings_.underlayer_color).
            SetStrokeWidth(render_settings_.underlayer_width).
            SetStrokeLineCap(svg::StrokeLineCap::ROUND).
            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        stops_name_and_underlayer.push_back(cur_stop_underlayer);
        stops_name_and_underlayer.push_back(cur_stop_name);
    }
    return stops_name_and_underlayer;
}

svg::Document MapRenderer::MakeSVGDocument(const std::set<const transport::Bus*, transport::BusComparator>& sorted_buses) const {
    svg::Document bus_map;
    const SphereProjector& sphere_projector = MakeSphereProjector(GetAllBusesStopsCoordinates(sorted_buses));
    for (const svg::Polyline& bus_line : GenerateBusesLines(sorted_buses, sphere_projector)) {
        bus_map.Add(bus_line);
    }
    for (const svg::Text& bus_name_and_underlayer : GenerateBusesNames(sorted_buses, sphere_projector)) {
        bus_map.Add(bus_name_and_underlayer);
    }
    const std::set<const transport::Stop*, transport::StopComparator>& sorted_stops = GetSortedStops(sorted_buses);
    for (const svg::Circle& stop_circle : GenerateStopsCircles(sorted_stops, sphere_projector)) {
        bus_map.Add(stop_circle);
    }
    for (const svg::Text& stop_name_and_underlayer : GenerateStopsNames(sorted_stops, sphere_projector)) {
        bus_map.Add(stop_name_and_underlayer);
    }
    return bus_map;
}

} // namespace renderer