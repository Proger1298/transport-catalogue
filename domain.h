#pragma once

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

#include "geo.h"

#include <string>
#include <vector>

namespace transport {

struct Stop {
	std::string stop_name;
	geo::Coordinates coordinates;
};

struct Bus {
	std::string bus_name;
	std::vector<const Stop*> stops;
	bool is_roundtrip;
};

struct BusInfo {
	size_t stop_count;
	size_t unique_stop_count;
	int route_length;
	double curvature;
};

struct BusComparator {
	bool operator()(const Bus* lhs, const Bus* rhs) const {
		return lhs->bus_name < rhs->bus_name;
	}
};
/*
struct StopHasher {
    std::size_t operator()(const Stop* stop) const noexcept {
        return std::hash<std::string>{}(stop->stop_name)
		+ std::hash<double>{}(stop->coordinates.lat) * 109
		+ std::hash<double>{}(stop->coordinates.lng) * 109 * 109;
	}
};
*/
struct StopComparator {
	bool operator()(const Stop* lhs, const Stop* rhs) const {
		return lhs->stop_name < rhs->stop_name;
	}
};

}