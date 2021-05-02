#pragma once

#include <memory>
#include <map>
#include <algorithm>

#include "transport_catalogue.h"
#include "map_renderer.h"


class RequestHandler {
public:
    using DistanceBetweenStops = std::unordered_map<std::pair<database::StopInfo, database::StopInfo>, int,
            database::StopInfoHasherPair>;

    RequestHandler(database::transport_catalogue& db, renderer::MapRenderer& renderer);

    // Возвращает маршруты, проходящие через
    const std::set<std::string>* GetBusesByStop(std::string_view stop_name) const;

    void AddStopsToDatabase(domain::Stop& stop);

    void AddRouteToDatabase(domain::Bus& buses_);

    void AddRouteToDatabaseFromSerializeData(domain::Bus& bus);

    void AddDistanceBetweenStopsToDatabase(std::string_view stop1, std::string_view stop2, unsigned int length);

    database::StopInfo GetStopByName(std::string_view stop);

    database::BusInfo GetBusByName(std::string_view bus);

    unsigned int GetDistance(std::string_view from, std::string_view to) const;

    svg::Document RenderMap();

    const std::deque<domain::Stop>& GetStops() const;

    const std::unordered_map<std::string_view, database::BusInfo>& GetRoutes() const;

    const DistanceBetweenStops& GetDistance() const;

    renderer::MapRenderer& GetMap() const;

    const size_t GetIndexStop(std::string_view name) const;

    const size_t GetIndexBus(std::string_view name) const;

    const domain::Stop* GetStopName(size_t index) const;

    const domain::Bus& GetBusName(size_t index) const;

    const std::deque<domain::Bus>& GetBusNumbers() const;

    void BuildMap();

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    database::transport_catalogue& db_;
    renderer::MapRenderer& renderer_;
    std::set<std::string> data_for_map_;
};
