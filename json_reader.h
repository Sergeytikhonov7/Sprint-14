#pragma once

#include <memory>
#include <fstream>

#include " json_builder.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"


class JsonReader {
public:
    using StopsDistance = std::unordered_map<std::shared_ptr<std::string>, std::unordered_map<std::shared_ptr<std::string>, unsigned int>>;

    JsonReader();

    router::TransportRouter
    ReadJsonDocument(std::istream& input, RequestHandler& requestHandler);

    void WriteStatistic(std::ostream& out, RequestHandler& requestHandler, router::TransportRouter& transport_router);

    void ReadFileNameAndStatRequest(std::istream& input);

    std::string GetFileName() {
        return filename_;
    }


private:
    StopsDistance distance_;
    json::Dict root_;
    json::Array stops_request_;
    json::Array stat_request_;
    json::Dict render_settings_;
    json::Dict routing_settings_;
    json::Array responses_;
    std::string filename_;

    void ParseDistance(RequestHandler& requestHandler);

    void ParseStops(RequestHandler& requestHandler, const json::Dict& dict);

    void ParseRoutes(RequestHandler& requestHandler, const json::Dict& dict);

    json::Node PrintStat(int id, const std::set<std::string>* stop);

    json::Node PrintErrorMessage(int id);

    json::Node PrintStatRoute(int id, const domain::Bus* route);

    void ParseRenderSettings(const std::map<std::string, json::Node>& settings, renderer::MapRenderer& renderer);

    std::vector<svg::Color> GetColor(const std::vector<json::Node>& colors);

    svg::Color GetCurrentColor(const json::Node& color);

    void CalculateMinMaxCoord(renderer::MapRenderer& renderer, const domain::Stop* stop);

};