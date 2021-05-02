#pragma once

#include <vector>

#include "request_handler.h"
#include "router.h"
#include " json_builder.h"

namespace router {
    struct BusActivity {
        std::string type;
        std::string_view name;
        std::string_view from;
        double time;
        unsigned int span_count = 0;
    };

    constexpr double KPH_TO_MPM = 1000.0 / 60.0;

    struct RoutingSettings {
        unsigned bus_wait_time = 0;
        double bus_velocity = 0;

        RoutingSettings() = default;

        RoutingSettings(unsigned int busWaitTime, double busVelocity);
    };

    class TransportRouter {
    public:
        TransportRouter();

        TransportRouter(const RequestHandler& requestHandler, RoutingSettings& routingSettings);

        json::Node GetRouteInfo(std::string_view from, std::string_view to, int request_id) const;

        const RoutingSettings& GetRoutingSettings() const;

        const std::shared_ptr<graph::DirectedWeightedGraph<double>>& GetGraph() const;

        const std::shared_ptr<graph::Router<double>>& GetRouter() const;

        const std::map<std::string_view, size_t>& GetStopNameToVertexId() const;

        const std::map<size_t, BusActivity>& GetActivity() const;

        void SetRouter(const std::shared_ptr<graph::Router<double>>& router);

        void InsertActivity(const size_t id, const BusActivity& bus_activity);

        void BuildMapStopToVertexId(const RequestHandler& requestHandler);

        void SetRoutingSettings(uint32_t wait_time, double bus_velocity);

    private:
        RoutingSettings routingSettings_;
        std::shared_ptr<graph::DirectedWeightedGraph<double>> graph_;
        std::shared_ptr<graph::Router<double>> router_;
        std::map<std::string_view, size_t> stop_name_to_vertexId_;
        std::map<size_t, BusActivity> activity_;
    };
}


