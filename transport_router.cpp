#include <utility>

#include "transport_router.h"

namespace transport {

    void TransportRouter::AddStopsToGraph(
        const std::set<const Stop*, StopComparator>& sorted_stops
    ) {
        graph::VertexId vertex_id = 0;
        graph::EdgeId cur_edge_id = 0;

        for (const Stop* stop : sorted_stops) {
            stop_to_vertexes_ids_[stop] = StopVertexes{vertex_id++, vertex_id++};
            cur_edge_id = graph_.AddEdge({
                stop_to_vertexes_ids_.at(stop).in,
                stop_to_vertexes_ids_.at(stop).out,
                static_cast<double>(route_settings_.bus_wait_time)
            });
            edge_id_to_edge_info_[cur_edge_id] = WaitEdgeInfo {
                stop,
                static_cast<double>(route_settings_.bus_wait_time)
            };
        }
    }

    void TransportRouter::AddBusesToGraph(
        const TransportCatalogue& catalogue
    ) {
        std::set<const Bus*, BusComparator> sorted_buses = catalogue.GetBusesSortedByName();
        graph::EdgeId cur_edge_id = 0;

        for (const Bus* bus : sorted_buses) {
            const std::vector<const Stop*>& cur_bus_stops = bus->stops;
            size_t cur_bus_stops_amount = cur_bus_stops.size();
            for (size_t from = 0; from < cur_bus_stops_amount; ++from) {
                const Stop* stop_from = cur_bus_stops.at(from);
                for (size_t to = from + 1; to < cur_bus_stops_amount; ++to) {
                    const Stop* stop_to = cur_bus_stops.at(to);
                    int cur_dist_between_stops = 0;
                    int reverse_dist_between_stops = 0;
                    for (size_t local = from + 1; local <= to; ++local) {
                        cur_dist_between_stops += catalogue.GetDistanceBetweenStops(
                            cur_bus_stops.at(local - 1),
                            cur_bus_stops.at(local)
                        );
                        reverse_dist_between_stops += catalogue.GetDistanceBetweenStops(
                            cur_bus_stops.at(local),
                            cur_bus_stops.at(local - 1)
                        );
                    }
                    cur_edge_id = graph_.AddEdge({
                        stop_to_vertexes_ids_.at(stop_from).out,
                        stop_to_vertexes_ids_.at(stop_to).in,
                        static_cast<double>(cur_dist_between_stops) / (route_settings_.bus_velocity * FROM_KM_H_TO_M_MIN)
                    });
                    edge_id_to_edge_info_[cur_edge_id] = BusEdgeInfo{
                        bus,
                        to - from,
                        static_cast<double>(cur_dist_between_stops) / (route_settings_.bus_velocity * FROM_KM_H_TO_M_MIN)
                    };
                    if (!bus->is_roundtrip) {
                        cur_edge_id = graph_.AddEdge({
                            stop_to_vertexes_ids_.at(stop_to).out,
                            stop_to_vertexes_ids_.at(stop_from).in,
                            static_cast<double>(reverse_dist_between_stops) / (route_settings_.bus_velocity * FROM_KM_H_TO_M_MIN)
                        });
                        edge_id_to_edge_info_[cur_edge_id] = BusEdgeInfo{
                            bus,
                            to - from,
                            static_cast<double>(cur_dist_between_stops) / (route_settings_.bus_velocity * FROM_KM_H_TO_M_MIN)
                        };
                    }
                }
            }
        }
    }

    TransportRouter::CompleteRouteInfo TransportRouter::FindOptimalRoute(
        const Stop* stop_from,
        const Stop* stop_to
    ) const {
        const std::optional<graph::Router<double>::RouteInfo> graph_route_info = router_->BuildRoute(
            stop_to_vertexes_ids_.at(stop_from).in,
            stop_to_vertexes_ids_.at(stop_to).in
        );
        if (!graph_route_info.has_value()) {
            return std::nullopt;
        }
        std::vector<TransportRouter::EdgeInfo> optimal_route;
        optimal_route.reserve(graph_route_info.value().edges.size());
        for (graph::EdgeId edge_id : graph_route_info.value().edges) {
            optimal_route.push_back(edge_id_to_edge_info_.at(edge_id));
        }
        return std::pair{optimal_route, graph_route_info.value().weight};
    }
} // namespace transport