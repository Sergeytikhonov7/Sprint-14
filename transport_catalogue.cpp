#include <algorithm>
#include "transport_catalogue.h"

namespace database {

    transport_catalogue::transport_catalogue() {
    }

    void transport_catalogue::AddStop(domain::Stop& stop) {
        auto current = &stop_name_.emplace_back(std::move(stop));
        index_stop_[current->name_] = stop_name_.size() - 1;
        stops_[current->name_] = current;
        buses_[current->name_];
    }

    void transport_catalogue::AddRoute(domain::Bus& busRoute) {
        auto route = &buses_number_.emplace_back(std::move(busRoute));
        index_bus_[route->number_] = buses_number_.size() - 1;
        routes_[route->number_] = route;
    }

    BusInfo transport_catalogue::FindRouteByName(std::string_view number) const {
        if (routes_.count(number) == 0) {
            return nullptr;
        }
        return routes_.at(number);
    }

    StopInfo transport_catalogue::FindStopByName(std::string_view stop) const {
        if (stops_.count(stop) == 0) {
            return nullptr;
        }
        return stops_.at(stop);
    }

    void transport_catalogue::AddBusesToStop(const domain::Bus& busRoute) {
        for (const auto& stop : busRoute.stops_) {
            buses_[stop->name_].insert(busRoute.number_);
        }
    }

    const std::set<std::string>* transport_catalogue::GetBusesByNameStop(std::string_view stop_name) const {
        if (buses_.count(stop_name) == 0) {
            return nullptr;
        }
        return &buses_.at(stop_name);
    }

    void transport_catalogue::AddDistanceBetweenStops(StopInfo from, StopInfo to, unsigned int length) {
        distance_[std::make_pair(from, to)] = length;
    }

    unsigned int transport_catalogue::GetDistanceBetweenStops(StopInfo stop1, StopInfo stop2) {
        if (distance_.count(std::make_pair(stop1, stop2)) == 0 && distance_.count(std::make_pair(stop2, stop1)) == 0) {
            return 0;
        }
        if (distance_.count(std::make_pair(stop1, stop2)) == 0) {
            return distance_.at(std::make_pair(stop2, stop1));
        }
        return distance_.at(std::make_pair(stop1, stop2));
    }

    const std::deque<domain::Stop>& transport_catalogue::GetStops() const {
        return stop_name_;
    }

    const Routes& transport_catalogue::GetRoutes() const {
        return routes_;
    }

    const DistanceBetweenStops& transport_catalogue::GetDistance() const {
        return distance_;
    }

    const size_t transport_catalogue::GetStopIndex(std::string_view name) const {
        auto it = std::find_if(stop_name_.begin(), stop_name_.end(),
                               [&](const domain::Stop& stop) { return stop.name_ == std::string(name); });
        return std::distance(stop_name_.begin(), it);
    }

    const size_t transport_catalogue::GetBusIndex(std::string_view name) const {
        return index_bus_.at(name);
    }

    const domain::Stop& transport_catalogue::GetStopByIndex(size_t index) const {
        return stop_name_.at(index);
    }

    const domain::Bus& transport_catalogue::GetBusByIndex(size_t index) const {
        return buses_number_.at(index);
    }

    const std::deque<domain::Bus>& transport_catalogue::GetBusesNumber() const {
        return buses_number_;
    }
}