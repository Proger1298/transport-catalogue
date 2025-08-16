#include "transport_catalogue.h"

#include <iostream>
#include <stdexcept>
#include <unordered_set>

namespace transport{

    void TransportCatalogue::AddStop(const Stop& stop) {
        stops_.push_front(stop);
        stopname_to_stop_[stops_.front().stop_name] = &stops_.front();
    }

    const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
        auto it = stopname_to_stop_.find(stop_name);
        return it != stopname_to_stop_.end() ? it->second : nullptr;
    }

    void TransportCatalogue::AddBus(const Bus& bus) {
        buses_.push_front(bus);
        busname_to_bus_[buses_.front().bus_name] = &buses_.front();
        for (const Stop* stop : buses_.front().stops) {
            stop_to_buses_[stop].insert(&buses_.front());
        }
    }

    const Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
        auto it = busname_to_bus_.find(bus_name);
        return it != busname_to_bus_.end() ? it->second : nullptr;
    }

    BusInfo TransportCatalogue::GetBusInfo(const Bus* bus) const {
        using namespace std::literals;
        if (!bus) {
            throw std::invalid_argument("Bus not found"s);
        }
        std::unordered_set<const Stop*> unique_stops;
        int route_length = 0;
        double geo_length = 0.0;
        for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
            unique_stops.insert(bus->stops[i]);
            route_length += GetDistanceBetweenStops(bus->stops[i], bus->stops[i+1]);
            geo_length += geo::ComputeDistance(bus->stops[i]->coordinates, bus->stops[i+1]->coordinates);
        }
        unique_stops.insert(bus->stops.back());
        if (!bus->is_roundtrip) {
            for (size_t i = bus->stops.size() - 1; i > 0; --i) {
                route_length += GetDistanceBetweenStops(bus->stops[i], bus->stops[i - 1]);
                geo_length += geo::ComputeDistance(bus->stops[i]->coordinates, bus->stops[i - 1]->coordinates);
            }
        }
        return {(bus->is_roundtrip ? bus->stops.size() : bus->stops.size() * 2 - 1), unique_stops.size(), route_length, route_length/geo_length};
    }

    void TransportCatalogue::SetDistanceBetweenStops(const Stop* from, const Stop* to, int distance) {
        stop_to_stop_distances_[{from, to}] = distance;
    }

    int TransportCatalogue::GetDistanceBetweenStops(const Stop* stop_1, const Stop* stop_2) const {
        auto it = stop_to_stop_distances_.find({stop_1, stop_2});
        if (it == stop_to_stop_distances_.end()) {
            it = stop_to_stop_distances_.find({stop_2, stop_1});
        }
        return it != stop_to_stop_distances_.end() ? it->second : 0;
    }

    std::set<const Bus*, BusComparator> TransportCatalogue::GetStopToBuses(const Stop* stop) const {
        using namespace std::literals;
        if (!stop) {
            throw std::invalid_argument("Stop not found"s);
        }
        if (stop_to_buses_.count(stop)) {
            return stop_to_buses_.at(stop);
        }
        return {};
    }

    const std::set<const Bus*, BusComparator> TransportCatalogue::GetBusesSortedByName() const {
        std::set<const Bus*, BusComparator> sorted_buses;
        for (const Bus& bus : buses_) {
            if (bus.stops.size() > 0) {
                sorted_buses.insert(&bus);
            }
        }
        return sorted_buses;
    }

    const std::set<const Stop*, StopComparator> TransportCatalogue::GetStopsSortedByName() const {
        std::set<const Stop*, StopComparator> sorted_stops;
        for (const Stop& stop : stops_) {
            sorted_stops.insert(&stop);
        }
        return sorted_stops;
    }

} // namespace transport