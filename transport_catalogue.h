#pragma once

#include "domain.h"

#include <forward_list>
#include <map>
#include <set>
#include <string_view>
#include <unordered_map>

namespace transport {

struct StopToStopHasher {
    size_t operator() (const std::pair<const Stop*, const Stop*>& stop_to_stop) const {
		size_t hash_1 = (
			static_cast<size_t>(stop_to_stop.first->coordinates.lat) *
			static_cast<size_t>(stop_to_stop.first->coordinates.lng) +
			str_hasher_(stop_to_stop.first->stop_name)
		);
		size_t hash_2 = (
			static_cast<size_t>(stop_to_stop.second->coordinates.lat) *
			static_cast<size_t>(stop_to_stop.second->coordinates.lng) +
			str_hasher_(stop_to_stop.second->stop_name)
		);

		return hash_1 + hash_2 * 109;
    }
private:
	std::hash<std::string> str_hasher_;
};

class TransportCatalogue {
public:
	void AddStop(const Stop& stop);
	const Stop* FindStop(std::string_view stop_name) const;
	void AddBus(const Bus& bus);
	const Bus* FindBus(std::string_view bus_name) const;
	BusInfo GetBusInfo(const Bus* bus) const;
	void SetDistanceBetweenStops(const Stop* from, const Stop* to, int distance);
    int GetDistanceBetweenStops(const Stop* stop_1, const Stop* stop_2) const;
	std::set<const Bus*, BusComparator> GetStopToBuses(const Stop* stop) const;
	const std::set<const Bus*, BusComparator> GetBusesSortedByName() const;
	const std::set<const Stop*, StopComparator> GetStopsSortedByName() const;
private:
	std::forward_list<Stop> stops_;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
	std::forward_list<Bus> buses_;
	std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
	std::unordered_map<
		std::pair<const Stop*, const Stop*>,
		int,
		StopToStopHasher
	> stop_to_stop_distances_;
	std::unordered_map<
		const Stop*,
		std::set<const Bus*, BusComparator>
	> stop_to_buses_;
};

} // namespace transport