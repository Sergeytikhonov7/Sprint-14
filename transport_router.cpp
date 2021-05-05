#include "transport_router.h"


router::RoutingSettings::RoutingSettings(unsigned int busWaitTime, double speed) : bus_wait_time(busWaitTime),
                                                                                   bus_velocity(speed * KPH_TO_MPM) {
}

router::TransportRouter::TransportRouter(const RequestHandler& requestHandler, router::RoutingSettings& routingSettings)
        : routingSettings_(routingSettings) {
    graph_ = std::make_unique<graph::DirectedWeightedGraph<double >>(requestHandler.GetStops().size());
    router::TransportRouter::BuildMapStopToVertexId(requestHandler);
    for (const auto& bus : requestHandler.GetRoutes()) {
        for (size_t i = 0; i < bus.second->stops_count; ++i) {
            double time_ = routingSettings_.bus_wait_time;
            unsigned int span_count_ = 0;
            for (size_t j = i + 1; j < bus.second->stops_count; ++j) {
                time_ += requestHandler.GetDistance(bus.second->stops_[j - 1]->name_, bus.second->stops_[j]->name_) /
                         routingSettings_.bus_velocity;
                size_t id = graph_->AddEdge(graph::Edge<double>{stop_name_to_vertexId_[bus.second->stops_[i]->name_],
                                                                stop_name_to_vertexId_[bus.second->stops_[j]->name_],
                                                                time_});
                activity_[id] = BusActivity{"Bus", bus.second->number_, bus.second->stops_[i]->name_,
                                            graph_.get()->GetEdge(id).weight,
                                            ++span_count_};
            }
        }
    }
    router_ = std::make_unique<graph::Router<double >>(*graph_);
}

json::Node router::TransportRouter::GetRouteInfo(std::string_view from, std::string_view to, int request_id) const {
    using namespace std::literals;
    auto route = router_.get()->BuildRoute(stop_name_to_vertexId_.at(from), stop_name_to_vertexId_.at(to));
    if (route.has_value()) {
        json::Array result;
        for (const auto& c : route.value().edges) {
            result.push_back(json::Builder{}.StartDict().
                    Key("stop_name"s).Value(std::move(std::string(activity_.at(c).from))).
                    Key("time"s).Value(int(routingSettings_.bus_wait_time)).
                    Key("type"s).Value("Wait"s).EndDict().Build());
            result.push_back(json::Builder{}.StartDict().
                    Key("bus"s).Value(std::move(std::string(activity_.at(c).name))).
                    Key("span_count"s).Value(int(activity_.at(c).span_count)).
                    Key("time"s).Value(activity_.at(c).time - routingSettings_.bus_wait_time).
                    Key("type"s).Value("Bus"s).EndDict().Build());
        }
        return json::Builder{}.StartDict().Key("items"s).Value(result).
                Key("request_id"s).Value(request_id).
                Key("total_time"s).Value(route.value().weight).
                EndDict().Build();
    } else {
        return json::Builder{}.StartDict().Key("error_message"s).Value("not found"s).
                Key("request_id"s).Value(request_id).
                EndDict().Build();
    }
}

void router::TransportRouter::BuildMapStopToVertexId(const RequestHandler& requestHandler) {
    const auto& stops = requestHandler.GetStops();
    for (size_t i = 0; i < stops.size(); ++i) {
        if (stop_name_to_vertexId_.count(stops[i].name_) != 1) {
            stop_name_to_vertexId_[stops[i].name_] = i;
        }
    }
}

const router::RoutingSettings& router::TransportRouter::GetRoutingSettings() const {
    return routingSettings_;
}

const std::shared_ptr<graph::DirectedWeightedGraph<double>>& router::TransportRouter::GetGraph() const {
    return graph_;
}

const std::shared_ptr<graph::Router<double>>& router::TransportRouter::GetRouter() const {
    return router_;
}

const std::map<std::string_view, size_t>& router::TransportRouter::GetStopNameToVertexId() const {
    return stop_name_to_vertexId_;
}

const std::map<size_t, router::BusActivity>& router::TransportRouter::GetActivity() const {
    return activity_;
}

void router::TransportRouter::SetRouter(const std::shared_ptr<graph::Router<double>>& router) {
    router_ = router;
}

void router::TransportRouter::InsertActivity(const size_t id, const router::BusActivity& bus_activity) {
    activity_[id] = bus_activity;
}

router::TransportRouter::TransportRouter() : graph_(std::make_shared<graph::DirectedWeightedGraph<double>>()),
                                             router_(std::make_shared<graph::Router<double>>(*graph_)) {

}

void router::TransportRouter::SetRoutingSettings(uint32_t wait_time, double bus_velocity) {
    routingSettings_.bus_wait_time = wait_time;
    routingSettings_.bus_velocity = bus_velocity;
}




