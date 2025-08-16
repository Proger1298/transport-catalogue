#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <variant>

#include "router.h"
#include "transport_catalogue.h"

namespace transport
{
    struct TransportRouteSettings
    {
        int bus_wait_time = 0;
        double bus_velocity = 0;
    };

    class TransportRouter {
    public:

        struct WaitEdgeInfo {
            const Stop* stop;
            double bus_wait_time = 0;
        };

        struct BusEdgeInfo {
            const Bus* bus;
            size_t span_count;
            double time;
        };

        using EdgeInfo = std::variant<WaitEdgeInfo, BusEdgeInfo>;
        using CompleteRouteInfo = std::optional<std::pair<std::vector<EdgeInfo>, double>>;
        
        explicit TransportRouter(TransportRouteSettings route_settings, const TransportCatalogue& catalogue) 
        :route_settings_(std::move(route_settings))
        {
            std::set<const Stop*, StopComparator> sorted_stops = catalogue.GetStopsSortedByName();
            graph_ = graph::DirectedWeightedGraph<double>(2 * sorted_stops.size());

            AddStopsToGraph(sorted_stops);
            AddBusesToGraph(catalogue);

            router_ = std::make_unique<graph::Router<double>>(graph_);
        }
        
        CompleteRouteInfo FindOptimalRoute(
            const Stop* stop_from,
            const Stop* stop_to
        ) const;

    private:
        static constexpr double FROM_KM_H_TO_M_MIN = 100.0 / 6.0; // Константа для перевода из км/ч в м/мин

        struct StopVertexes {
            graph::VertexId in;
            graph::VertexId out;
        };

        TransportRouteSettings route_settings_;
        graph::DirectedWeightedGraph<double> graph_;
        std::unique_ptr<graph::Router<double>> router_;
        std::unordered_map<const Stop*, StopVertexes> stop_to_vertexes_ids_;
        std::unordered_map<graph::EdgeId, EdgeInfo> edge_id_to_edge_info_;

        void AddStopsToGraph(
            const std::set<const Stop*, StopComparator>& sorted_stops
        );

        void AddBusesToGraph(
            const TransportCatalogue& catalogue
        );
    };
    
} // namespace transport
