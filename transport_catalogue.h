#pragma once

#include <set>
#include <unordered_map>
#include <stdexcept>

#include "domain.h"


namespace database {

    using StopInfo = const domain::Stop*;

    using BusInfo = const domain::Bus*;

    using Routes = std::unordered_map<std::string_view, BusInfo>;

    struct StopInfoHasherPair {
        size_t operator()(const std::pair<StopInfo, StopInfo>& stops) const {
            return hasher(stops.first) + hasher(stops.second) * 37;
        }

    private:
        std::hash<const void*> hasher;
    };

    using DistanceBetweenStops = std::unordered_map<std::pair<StopInfo, StopInfo>, int, StopInfoHasherPair>;

    class transport_catalogue {
    public:

        explicit transport_catalogue();

        void AddStop(domain::Stop& stop);

        void AddRoute(domain::Bus& busRoute);

        void AddBusesToStop(const domain::Bus& busRoute);

        void AddDistanceBetweenStops(StopInfo from, StopInfo to, unsigned int length);

        BusInfo FindRouteByName(std::string_view number) const;

        StopInfo FindStopByName(std::string_view stop) const;

        const std::set<std::string>* GetBusesByNameStop(std::string_view stop_name) const;

        unsigned int GetDistanceBetweenStops(StopInfo stop1, StopInfo stop2);

        const std::deque<domain::Stop>& GetStops() const;

        const Routes& GetRoutes() const;

        const DistanceBetweenStops& GetDistance() const;

        const size_t GetStopIndex(std::string_view name) const;

        const size_t GetBusIndex(std::string_view name) const;

        const domain::Stop& GetStopByIndex(size_t index) const;

        const domain::Bus& GetBusByIndex(size_t index) const;

        const std::deque<domain::Bus>& GetBusesNumber() const;

    private:
        std::deque<domain::Stop> stop_name_;
        std::deque<domain::Bus> buses_number_;
        std::unordered_map<std::string_view, size_t> index_stop_;
        std::unordered_map<std::string_view, size_t> index_bus_;
        std::unordered_map<std::string_view, StopInfo> stops_;
        std::unordered_map<std::string_view, BusInfo> routes_;
        std::unordered_map<std::string_view, std::set<std::string>> buses_;
        DistanceBetweenStops distance_;
    };

    bool operator==(const domain::Stop& lhs, const domain::Stop& rhs);
}