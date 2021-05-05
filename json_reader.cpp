#include "json_reader.h"


JsonReader::JsonReader() {}


void JsonReader::ReadFileNameAndStatRequest(std::istream& input) {
    json::Document document = json::Load(input);
    root_ = document.GetRoot().AsDict();
    filename_ = root_.at("serialization_settings").AsDict().at("file").AsString();
    stat_request_ = root_.at("stat_requests").AsArray();
}

router::TransportRouter JsonReader::ReadJsonDocument(std::istream& input, RequestHandler& requestHandler) {
    json::Document document = json::Load(input);
    root_ = document.GetRoot().AsDict();
    filename_ = root_.at("serialization_settings").AsDict().at("file").AsString();;
    if (root_.count("base_requests") == 1) {
        stops_request_ = root_.at("base_requests").AsArray();
    }
    if (root_.count("render_settings") == 1) {
        render_settings_ = root_.at("render_settings").AsDict();
    }
    if (root_.count("routing_settings") == 1) {
        routing_settings_ = root_.at("routing_settings").AsDict();
    }

    for (size_t i = 0; i < stops_request_.size(); ++i) {
        if (stops_request_[i].AsDict().at("type").AsString() == "Stop") {
            ParseStops(requestHandler, stops_request_[i].AsDict());
        }
    }

    ParseDistance(requestHandler);
    for (size_t i = 0; i < stops_request_.size(); ++i) {
        if (stops_request_[i].AsDict().at("type").AsString() == "Bus") {
            ParseRoutes(requestHandler, stops_request_[i].AsDict());
        }
    }

    if (!render_settings_.empty()) {
        ParseRenderSettings(render_settings_, requestHandler.GetMap());
    }
    router::RoutingSettings routingSetting;

    if (!routing_settings_.empty()) {
        routingSetting = router::RoutingSettings(routing_settings_.at("bus_wait_time").AsInt(),
                                                 routing_settings_.at("bus_velocity").AsDouble());
    }
    requestHandler.BuildMap();
    return router::TransportRouter(requestHandler, routingSetting);
}

void JsonReader::WriteStatistic(std::ostream& out, RequestHandler& requestHandler,
                                router::TransportRouter& transport_router) {
    for (const auto& stat : stat_request_) {
        json::Dict dict = stat.AsDict();
        int id = dict.at("id").AsInt();
        if (dict.at("type").AsString() == "Stop") {
            auto stop = requestHandler.GetBusesByStop(std::move(dict.at("name").AsString()));
            stop ? responses_.push_back(std::move(PrintStat(id, stop))) : responses_.push_back(
                    std::move(PrintErrorMessage(id)));
        }
        if (dict.at("type").AsString() == "Bus") {
            auto route = requestHandler.GetBusByName(std::move(dict.at("name").AsString()));
            route ? responses_.push_back(std::move(PrintStatRoute(id, route))) : responses_.push_back(
                    std::move(PrintErrorMessage(id)));
        }
        if (dict.at("type").AsString() == "Map") {
            auto doc = requestHandler.RenderMap();
            std::stringstream svg;
            std::stringstream svg_out;
            doc.Render(svg);
            responses_.push_back(
                    std::move(json::Builder{}.StartDict().Key("map").Value(svg.str()).Key("request_id").Value(
                            id).EndDict().Build()));
        }
        if (dict.at("type").AsString() == "Route") {
            responses_.push_back(
                    transport_router.GetRouteInfo(dict.at("from").AsString(), dict.at("to").AsString(), id));
        }
    }
    json::Print(json::Document(json::Builder{}.Value(responses_).Build()), out);

}

void JsonReader::ParseRoutes(RequestHandler& requestHandler, const json::Dict& dict) {
    domain::Bus bus;
    std::set<std::string> uniq_stops;
    bus.number_ = std::move(dict.at("name").AsString());
    bus.is_roundtrip = dict.at("is_roundtrip").AsBool();
    for (const auto& stop : dict.at("stops_").AsArray()) {
        uniq_stops.insert(stop.AsString());
        auto current_stop = requestHandler.GetStopByName(std::move(stop.AsString()));
        CalculateMinMaxCoord(requestHandler.GetMap(), current_stop);
        bus.stops_.push_back(current_stop);
    }
    if (!bus.stops_.empty()) {
        bus.last_stop_ = bus.stops_.back()->coordinates_;
    }
    if (!bus.is_roundtrip) {
        bus.stops_.insert(bus.stops_.end(), std::next(bus.stops_.rbegin()), bus.stops_.rend());
    }
    bus.stops_count = bus.stops_.size();
    bus.unique_stops_count_ = uniq_stops.size();
    requestHandler.AddRouteToDatabase(bus);
}

void JsonReader::ParseStops(RequestHandler& requestHandler, const json::Dict& dict) {
    domain::Stop stop{dict.at("name").AsString(), {dict.at("latitude").AsDouble(), dict.at("longitude").AsDouble()}};
    for (const auto&[key, value] : dict.at("road_distances").AsDict()) {
        distance_[std::make_shared<std::string>(stop.name_)][std::make_shared<std::string>(key)] = value.AsInt();
    }
    requestHandler.AddStopsToDatabase(stop);
}

void JsonReader::ParseDistance(RequestHandler& requestHandler) {
    for (const auto& from : distance_) {
        for (const auto&[to, distance] : from.second) {
            requestHandler.AddDistanceBetweenStopsToDatabase(*from.first, *to, distance);
        }
    }
}

void JsonReader::ParseRenderSettings(const std::map<std::string, json::Node>& settings,
                                     renderer::MapRenderer& renderer) {
    if (!settings.empty()) {
        renderer.GetRender().width_ = settings.at("width").AsDouble();
        renderer.GetRender().height_ = settings.at("height").AsDouble();
        renderer.GetRender().padding_ = settings.at("padding").AsDouble();
        renderer.GetRender().line_width_ = settings.at("line_width").AsDouble();
        renderer.GetRender().stop_radius_ = settings.at("stop_radius").AsDouble();
        renderer.GetRender().bus_label_font_size_ = settings.at("bus_label_font_size").AsInt();
        renderer.GetRender().stop_label_font_size_ = settings.at("stop_label_font_size").AsInt();
        renderer.GetRender().bus_label_offset_ = {settings.at("bus_label_offset").AsArray()[0].AsDouble(),
                                                  settings.at("bus_label_offset").AsArray()[1].AsDouble()};
        renderer.GetRender().stop_label_offset_ = {settings.at("stop_label_offset").AsArray()[0].AsDouble(),
                                                   settings.at("stop_label_offset").AsArray()[1].AsDouble()};
        renderer.GetRender().underlayer_width_ = settings.at("underlayer_width").AsDouble();

        renderer.GetRender().underlayer_color_ = GetCurrentColor(settings.at("underlayer_color"));
        renderer.GetRender().color_palette_ = std::move(GetColor(settings.at("color_palette").AsArray()));
    }
}

std::vector<svg::Color> JsonReader::GetColor(const std::vector<json::Node>& colors) {
    std::vector<svg::Color> result;
    for (const auto& color : colors) {
        result.emplace_back(GetCurrentColor(color));
    }
    return result;
}

void JsonReader::CalculateMinMaxCoord(renderer::MapRenderer& renderer, const domain::Stop* stop) {
    renderer.GetRender().min_lat_ = std::min(renderer.GetRender().min_lat_, stop->coordinates_.lat);
    renderer.GetRender().min_lon_ = std::min(renderer.GetRender().min_lon_, stop->coordinates_.lng);
    renderer.GetRender().max_lat_ = std::max(renderer.GetRender().max_lat_, stop->coordinates_.lat);
    renderer.GetRender().max_lon_ = std::max(renderer.GetRender().max_lon_, stop->coordinates_.lng);

}

svg::Color JsonReader::GetCurrentColor(const json::Node& color) {
    svg::Color result;
    if (color.IsString()) {
        result = std::move(color.AsString());
    } else if (color.IsArray()) {
        size_t size = color.AsArray().size();
        if (size == 3) {
            result = svg::Rgb(
                    static_cast<uint8_t>(color.AsArray()[0].AsInt()),
                    static_cast<uint8_t>(color.AsArray()[1].AsInt()),
                    static_cast<uint8_t>(color.AsArray()[2].AsInt()));
        }
        if (size == 4) {
            result = svg::Rgba(
                    static_cast<uint8_t>(color.AsArray()[0].AsInt()),
                    static_cast<uint8_t>(color.AsArray()[1].AsInt()),
                    static_cast<uint8_t>(color.AsArray()[2].AsInt()),
                    color.AsArray()[3].AsDouble());
        }
    }
    return result;
}


json::Node JsonReader::PrintStatRoute(int id, const domain::Bus* route) {
    return json::Builder{}.StartDict().Key("curvature").Value(route->real_distance_ / route->length_)
            .Key("request_id").Value(id)
            .Key("route_length").Value(int(route->real_distance_))
            .Key("stop_count").Value(int(route->stops_count))
            .Key("unique_stop_count").Value(int(route->unique_stops_count_)).EndDict().Build();
}

json::Node JsonReader::PrintErrorMessage(int id) {
    return json::Builder{}.StartDict().Key("request_id").Value(id)
            .Key("error_message").Value("not found").EndDict().Build();
}

json::Node JsonReader::PrintStat(int id, const std::set<std::string>* stop) {
    return json::Builder{}.StartDict().Key("buses").Value(json::Array{stop->begin(), stop->end()}).Key(
            "request_id").Value(id).EndDict().Build();
}