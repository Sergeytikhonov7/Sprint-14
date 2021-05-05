#include "request_handler.h"

RequestHandler::RequestHandler(database::transport_catalogue& db, renderer::MapRenderer& renderer) :
        db_(db), renderer_(renderer) {}

void RequestHandler::AddStopsToDatabase(domain::Stop& stop) {
    db_.AddStop(stop);
}

void RequestHandler::AddDistanceBetweenStopsToDatabase(std::string_view from, std::string_view to, unsigned int distance) {
    database::StopInfo stop_info1 = db_.FindStopByName(from);
    database::StopInfo stop_info2 = db_.FindStopByName(to);
    db_.AddDistanceBetweenStops(stop_info1, stop_info2, distance);
}

void RequestHandler::AddRouteToDatabase(domain::Bus& bus) {
    if (bus.stops_count > 0) {
        for (size_t idx = 0; idx < bus.stops_count - 1; ++idx) {
            bus.length_ += geo::ComputeDistance(bus.stops_[idx]->coordinates_, bus.stops_[idx + 1]->coordinates_);
            bus.real_distance_ += GetDistance(bus.stops_[idx]->name_, bus.stops_[idx + 1]->name_);
        }
    }
    if (bus.stops_count > 0) {
        data_for_map_.insert(bus.number_);
    }
    db_.AddBusesToStop(bus);
    db_.AddRoute(bus);
}

void RequestHandler::AddRouteToDatabaseFromSerializeData(domain::Bus& bus) {
    if (bus.stops_count > 0) {
        data_for_map_.insert(bus.number_);
    }
    db_.AddBusesToStop(bus);
    db_.AddRoute(bus);
}

database::StopInfo RequestHandler::GetStopByName(std::string_view stop) {
    return db_.FindStopByName(stop);
}

database::BusInfo RequestHandler::GetBusByName(std::string_view bus) {
    return db_.FindRouteByName(bus);
}

unsigned int RequestHandler::GetDistance(std::string_view stop1, std::string_view stop2) const {
    auto stopInfo1 = db_.FindStopByName(stop1);
    auto stopInfo2 = db_.FindStopByName(stop2);
    return db_.GetDistanceBetweenStops(stopInfo1, stopInfo2);
}

const std::set<std::string>* RequestHandler::GetBusesByStop(std::string_view stop_name) const {
    return db_.GetBusesByNameStop(stop_name);
}

void RequestHandler::BuildMap() {
    renderer_.CalculationCoef();
    std::set<const domain::Stop*, domain::StopComp> uniq_stops;
    for (const auto& number : data_for_map_) {
        auto bus = GetBusByName(number);
        if (!bus || bus->stops_count == 0) {
            continue;
        }
        uniq_stops.insert(bus->stops_.begin(), bus->stops_.end());
        renderer_.AddRouteToMap(bus);
    }
    renderer_.AddStopToMap(uniq_stops);
}

svg::Document RequestHandler::RenderMap() {
    svg::Document document;
    for (auto& polyline : renderer_.GetRender().polyline_) {
        document.Add(std::move(*polyline));
    }
    for (auto& text_polyline : renderer_.GetRender().text_polyline_) {
        document.Add(std::move(*text_polyline));
    }
    for (auto& circle : renderer_.GetRender().circle_) {
        document.Add(std::move(*circle));
    }
    for (auto& text_circle : renderer_.GetRender().text_circle_) {
        document.Add(std::move(*text_circle));
    }
    return document;
}

const std::deque<domain::Stop>& RequestHandler::GetStops() const {
    return db_.GetStops();
}

const std::unordered_map<std::string_view, database::BusInfo>& RequestHandler::GetRoutes() const {
    return db_.GetRoutes();
}

const RequestHandler::DistanceBetweenStops& RequestHandler::GetDistance() const {
    return db_.GetDistance();
}

renderer::MapRenderer& RequestHandler::GetMap() const {
    return renderer_;
}

const size_t RequestHandler::GetIndexStop(std::string_view name) const {
    return db_.GetStopIndex(name);
}

const size_t RequestHandler::GetIndexBus(std::string_view name) const {
    return db_.GetBusIndex(name);
}

const domain::Stop* RequestHandler::GetStopName(size_t index) const {
    return &db_.GetStopByIndex(index);
}

const domain::Bus& RequestHandler::GetBusName(size_t index) const {
    return db_.GetBusByIndex(index);
}

const std::deque<domain::Bus>& RequestHandler::GetBusNumbers() const {
    return db_.GetBusesNumber();
}
