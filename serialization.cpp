#include <fstream>
#include "serialization.h"

//-------Сериализация БД--------------

void Serialization(std::ostream& out, const RequestHandler& requestHandler, const router::TransportRouter& router) {
    serialization_data::TransportCatalogue data;
    for (const auto& stop: requestHandler.GetStops()) {
        StopSerialization(data, stop);
    }
    const auto render_settings = router.GetRoutingSettings();
    data.mutable_transport_router()->mutable_router_settings()->set_bus_wait_time(render_settings.bus_wait_time);
    data.mutable_transport_router()->mutable_router_settings()->set_bus_velocity(render_settings.bus_velocity);
    RouteSerialization(data, requestHandler);
    DistanceSerialization(data, requestHandler);
    MapSerialization(requestHandler, data);
    GraphSerialization(router, data);
    InternalDataSerialization(router, data);
    ActivitySerialization(requestHandler, router, data);
    data.SerializeToOstream(&out);
}

void ActivitySerialization(const RequestHandler& requestHandler, const router::TransportRouter& router,
                           serialization_data::TransportCatalogue& data) {
    for (const auto& activity: router.GetActivity()) {
        auto current_activity = data.mutable_transport_router()->add_activity();
        current_activity->set_number(requestHandler.GetIndexBus(activity.second.name));
        current_activity->set_from(requestHandler.GetIndexStop(activity.second.from));
        current_activity->set_time(activity.second.time);
        current_activity->set_span_count(activity.second.span_count);
        current_activity->set_vertexid(activity.first);
    }
}

void InternalDataSerialization(const router::TransportRouter& router, serialization_data::TransportCatalogue& data) {
    for (const auto& internal_data: router.GetRouter()->GetRoutesInternalData()) {
        auto current_data = data.mutable_transport_router()->add_data();
        for (const auto& internal: internal_data) {
            if (internal.has_value()) {
                auto current_internal = current_data->add_internal_data();
                current_internal->set_has_internal_data(true);
                if (internal->prev_edge.has_value()) {
                    current_internal->set_weight(internal->weight);
                    current_internal->set_prev_edge(internal->prev_edge.value());
                    current_internal->set_has_prev_edge_(true);
                } else {
                    current_internal->set_weight(internal->weight);
                }
            } else {
                current_data->add_internal_data();
            }
        }
    }
}

void GraphSerialization(const router::TransportRouter& router, serialization_data::TransportCatalogue& data) {
    for (const auto& id: router.GetGraph()->GetEdges()) {
        auto current_edge = data.mutable_transport_router()->mutable_graph()->mutable_edges()->add_edge_list();
        current_edge->set_from(id.from);
        current_edge->set_to(id.to);
        current_edge->set_weight(id.weight);
    }

    for (const auto& incident: router.GetGraph()->GetIncidenceLists()) {
        auto current_list = data.mutable_transport_router()->mutable_graph()->mutable_lists()->add_incident_lists();
        for (const auto& id: incident) {
            current_list->add_edgeid(id);
        }
    }
}

void MapSerialization(const RequestHandler& requestHandler, serialization_data::TransportCatalogue& data) {
    const auto render_map = requestHandler.GetMap().GetRender();
    data.mutable_map()->set_bus_label_font_size(render_map.bus_label_font_size_);
    data.mutable_map()->set_stop_label_font_size(render_map.stop_label_font_size_);
    data.mutable_map()->set_width(render_map.width_);
    data.mutable_map()->set_height(render_map.height_);
    data.mutable_map()->set_padding(render_map.padding_);
    data.mutable_map()->set_line_width(render_map.line_width_);
    data.mutable_map()->set_stop_radius(render_map.stop_radius_);
    data.mutable_map()->set_underlayer_width(render_map.underlayer_width_);
    data.mutable_map()->set_min_lat(render_map.min_lat_);
    data.mutable_map()->set_min_lon(render_map.min_lon_);
    data.mutable_map()->set_max_lat(render_map.max_lat_);
    data.mutable_map()->set_max_lon(render_map.max_lon_);
    data.mutable_map()->set_zoom_coef(render_map.zoom_coef_);
    data.mutable_map()->mutable_bus_label_offset_()->set_x(render_map.bus_label_offset_.x);
    data.mutable_map()->mutable_bus_label_offset_()->set_y(render_map.bus_label_offset_.y);
    data.mutable_map()->mutable_stop_label_offset_()->set_x(render_map.stop_label_offset_.x);
    data.mutable_map()->mutable_stop_label_offset_()->set_y(render_map.stop_label_offset_.y);
    SerializationColor(data.mutable_map()->mutable_underlayer_color(), render_map.underlayer_color_);
    for (const svg::Color& palette: render_map.color_palette_) {
        SerializationColor(data.mutable_map()->add_color_palette(), palette);
    }
    for (const auto& polyline: render_map.polyline_) {
        PolylineSerialization(data, polyline);
    }
    for (const auto& text: render_map.text_polyline_) {
        TextPolylineSerialization(data, text);
    }
    for (const auto& circle: render_map.circle_) {
        CircleSerialization(data, circle);
    }
    for (const auto& text_circle: render_map.text_circle_) {
        TextCircleSerialization(data, text_circle);
    }
}

void CircleSerialization(serialization_data::TransportCatalogue& data, const std::shared_ptr<svg::Circle>& circle) {
    auto current_circle = data.mutable_map()->add_circle();
    current_circle->mutable_center()->set_x(circle->getCenter().x);
    current_circle->mutable_center()->set_y(circle->getCenter().y);
    current_circle->set_radius(circle->getRadius());
    AttrSerialization(circle, current_circle);
}

void TextPolylineSerialization(serialization_data::TransportCatalogue& data, const std::shared_ptr<svg::Text>& text) {
    auto current_text = data.mutable_map()->add_text_polyline();
    current_text->mutable_position()->set_x(text->getPosition().x);
    current_text->mutable_position()->set_y(text->getPosition().y);
    current_text->mutable_offset()->set_x(text->getOffset().x);
    current_text->mutable_offset()->set_y(text->getOffset().y);
    current_text->set_font_size(text->getFontSize());
    if (text->getFontFamily().has_value()) {
        current_text->set_font_family(text->getFontFamily().value());
        current_text->set_has_font_family_(true);
    }
    if (text->getFontWeight().has_value()) {
        current_text->set_font_weight(text->getFontWeight().value());
        current_text->set_has_font_weight_(true);
    }
    current_text->set_data(text->getData());
    AttrSerialization(text, current_text);
}

void TextCircleSerialization(serialization_data::TransportCatalogue& data, const std::shared_ptr<svg::Text>& text) {
    auto current_text = data.mutable_map()->add_text_circle();
    current_text->mutable_position()->set_x(text->getPosition().x);
    current_text->mutable_position()->set_y(text->getPosition().y);
    current_text->mutable_offset()->set_x(text->getOffset().x);
    current_text->mutable_offset()->set_y(text->getOffset().y);
    current_text->set_font_size(text->getFontSize());
    if (text->getFontFamily().has_value()) {
        current_text->set_font_family(text->getFontFamily().value());
        current_text->set_has_font_family_(true);
    }
    if (text->getFontWeight().has_value()) {
        current_text->set_font_weight(text->getFontWeight().value());
        current_text->set_has_font_weight_(true);
    }
    current_text->set_data(text->getData());
    AttrSerialization(text, current_text);
}

void PolylineSerialization(serialization_data::TransportCatalogue& data, const std::shared_ptr<svg::Polyline>& polyline) {
    auto current_polyline = data.mutable_map()->add_polyline();
    for (const auto& point: polyline->getPoints()) {
        auto points = current_polyline->add_points();
        points->set_x(point.x);
        points->set_y(point.y);
    }
    AttrSerialization(polyline, current_polyline);
}

void SerializationColor(serialization_data::Color* data, const svg::Color& color) {
    if (std::holds_alternative<std::string>(color)) {
        data->set_color(std::get<std::string>(color));
        data->set_has_color_(true);
    }
    if (std::holds_alternative<svg::Rgb>(color)) {
        svg::Rgb rgb = std::get<svg::Rgb>(color);
        data->mutable_rgb()->set_red(rgb.red);
        data->mutable_rgb()->set_green(rgb.green);
        data->mutable_rgb()->set_blue(rgb.blue);
        data->set_has_rgb_(true);
    }
    if (std::holds_alternative<svg::Rgba>(color)) {
        svg::Rgba rgb = std::get<svg::Rgba>(color);
        data->mutable_rgba()->set_red(rgb.red);
        data->mutable_rgba()->set_green(rgb.green);
        data->mutable_rgba()->set_blue(rgb.blue);
        data->mutable_rgba()->set_opacity(rgb.opacity);
        data->set_has_rgba_(true);
    }
}

void DistanceSerialization(serialization_data::TransportCatalogue& data,
                           const RequestHandler& requestHandler) {
    for (const auto&[stop, value]: requestHandler.GetDistance()) {
        auto current_stops = data.add_distance();
        current_stops->set_from(requestHandler.GetIndexStop(stop.first->name_));
        current_stops->set_to(requestHandler.GetIndexStop(stop.second->name_));
        current_stops->set_length(value);
    }
}

void StopSerialization(serialization_data::TransportCatalogue& data, const domain::Stop& stop) {
    auto current_stop = data.add_stops();
    current_stop->set_name(stop.name_);
    current_stop->mutable_coordinate()->set_lat(stop.coordinates_.lat);
    current_stop->mutable_coordinate()->set_lng(stop.coordinates_.lng);
}

void RouteSerialization(serialization_data::TransportCatalogue& data,
                        const RequestHandler& requestHandler) {
    for (const auto& route: requestHandler.GetBusNumbers()) {
        auto current_route = data.add_buses();
        current_route->set_is_round(route.is_roundtrip);
        current_route->set_number(route.number_);
        current_route->set_stops_count(route.stops_count);
        current_route->set_unique_count(route.unique_stops_count_);
        current_route->set_real_distance(route.real_distance_);
        current_route->set_length(route.length_);
        current_route->mutable_last_stop()->set_lat(route.last_stop_.lat);
        current_route->mutable_last_stop()->set_lat(route.last_stop_.lng);
        for (const auto& stop: route.stops_) {
            current_route->add_stops(requestHandler.GetIndexStop(stop->name_));
        }
    }
}

//-------Десериализация БД--------------

void Deserialization(std::istream& in, RequestHandler& requestHandler, router::TransportRouter& router) {
    serialization_data::TransportCatalogue data;
    if (data.ParseFromIstream(&in)) {
        for (const auto& stop: data.stops()) {
            StopDeserialization(requestHandler, stop);
        }
        for (const auto& distance: data.distance()) {
            requestHandler.AddDistanceBetweenStopsToDatabase(requestHandler.GetStopName(distance.from())->name_,
                                                             requestHandler.GetStopName(distance.to())->name_,
                                                             distance.length());
        }
        for (const auto& route: data.buses()) {
            RouteDeserialization(requestHandler, route);
        }
        router.SetRoutingSettings(data.transport_router().router_settings().bus_wait_time(),
                                  data.transport_router().router_settings().bus_velocity());
        GraphDeserialization(router, data);
        InternalDataDeserialization(router, data);
        ActivityDeserialization(requestHandler, router, data);
        router.BuildMapStopToVertexId(requestHandler);
        MapDeserialization(data, requestHandler.GetMap().GetRender());
    }
}

void ActivityDeserialization(const RequestHandler& requestHandler, router::TransportRouter& router,
                             const serialization_data::TransportCatalogue& data) {

    for (const auto& activity: data.transport_router().activity()) {
        router::BusActivity current_activity;
        current_activity.type = "Bus";
        current_activity.name = requestHandler.GetBusName(activity.number()).number_;
        current_activity.from = requestHandler.GetStopName(activity.from())->name_;
        current_activity.time = activity.time();
        current_activity.span_count = activity.span_count();
        router.InsertActivity(activity.vertexid(), current_activity);
    }
}

void InternalDataDeserialization(router::TransportRouter& router, const serialization_data::TransportCatalogue& data) {
    for (const auto& internal_data: data.transport_router().data()) {
        std::vector<std::optional<graph::RouteInternalData<double>>> tmp(internal_data.internal_data_size());
        size_t idx = 0;
        for (const auto& internal: internal_data.internal_data()) {
            if (internal.has_internal_data()) {
                if (internal.has_prev_edge_()) {
                    tmp[idx] = graph::RouteInternalData<double>{internal.weight(), internal.prev_edge()};
                } else {
                    tmp[idx] = graph::RouteInternalData<double>{internal.weight()};
                }
            }
            ++idx;
        }
        router.GetRouter()->SetRoutesInternalData(tmp);
    }
    graph::Router<double> built_router(*router.GetGraph(), router.GetRouter()->GetRoutesInternalData());
    router.SetRouter(std::make_shared<graph::Router<double>>(built_router));
}

void GraphDeserialization(router::TransportRouter& router, const serialization_data::TransportCatalogue& data) {
    for (const auto& edge: data.transport_router().graph().edges().edge_list()) {
        graph::Edge<double> current_edge;
        current_edge.from = edge.from();
        current_edge.to = edge.to();
        current_edge.weight = edge.weight();
        router.GetGraph()->SetEdge(current_edge);
    }
    for (const auto& incident: data.transport_router().graph().lists().incident_lists()) {
        std::vector<graph::EdgeId> edges;
        for (const auto& id: incident.edgeid()) {
            edges.push_back(id);
        }
        router.GetGraph()->SetIncidenceList(edges);
    }
}

void MapDeserialization(const serialization_data::TransportCatalogue& data, Render& map) {
    map.bus_label_font_size_ = data.map().bus_label_font_size();
    map.stop_label_font_size_ = data.map().stop_label_font_size();
    map.width_ = data.map().width();
    map.height_ = data.map().height();
    map.padding_ = data.map().padding();
    map.line_width_ = data.map().line_width();
    map.stop_radius_ = data.map().stop_radius();
    map.underlayer_width_ = data.map().underlayer_width();
    map.min_lat_ = data.map().min_lat();
    map.min_lon_ = data.map().max_lon();
    map.max_lat_ = data.map().max_lat();
    map.max_lon_ = data.map().max_lon();
    map.zoom_coef_ = data.map().zoom_coef();
    map.bus_label_offset_.x = data.map().bus_label_offset_().x();
    map.bus_label_offset_.y = data.map().bus_label_offset_().y();
    map.stop_label_offset_.x = data.map().stop_label_offset_().x();
    map.stop_label_offset_.y = data.map().stop_label_offset_().y();
    svg::Color* underlayer_color = &map.underlayer_color_;
    const serialization_data::Color* serializ_color = &data.map().underlayer_color();
    ColorDeserialization(underlayer_color, serializ_color);
    for (const auto& palette: data.map().color_palette()) {
        svg::Color tmp;
        ColorDeserialization(&tmp, &palette);
        map.color_palette_.push_back(std::move(tmp));
    }
    for (const auto& polyline: data.map().polyline()) {
        svg::Polyline tmp;
        PolylineDeserialization(polyline, tmp);
        map.polyline_.push_back(std::make_shared<svg::Polyline>(tmp));
    }
    for (const auto& text: data.map().text_polyline()) {
        svg::Text tmp;
        TextDeserialization(text, tmp);
        map.text_polyline_.push_back(std::make_shared<svg::Text>(tmp));
    }
    for (const auto& circle: data.map().circle()) {
        svg::Circle tmp;
        CircleDeserialization(circle, tmp);
        map.circle_.push_back(std::make_shared<svg::Circle>(tmp));
    }
    for (const auto& text_circle: data.map().text_circle()) {
        svg::Text tmp;
        TextDeserialization(text_circle, tmp);
        map.text_circle_.push_back(std::make_shared<svg::Text>(tmp));
    }
}

void CircleDeserialization(const serialization_data::Circle& circle, svg::Circle& tmp) {
    tmp.SetCenter({circle.center().x(), circle.center().y()});
    tmp.SetRadius(circle.radius());
    AttrDeserialization(circle, tmp);
}

void TextDeserialization(const serialization_data::Text& text, svg::Text& tmp) {
    tmp.SetPosition({text.position().x(), text.position().y()});
    tmp.SetOffset({text.offset().x(), text.offset().y()});
    tmp.SetFontSize(text.font_size());
    if (text.has_font_family_()) {
        tmp.SetFontFamily(text.font_family());
    }
    if (text.has_font_weight_()) {
        tmp.SetFontWeight(text.font_weight());
    }
    tmp.SetData(text.data());
    AttrDeserialization(text, tmp);
}

void PolylineDeserialization(const serialization_data::Polyline& polyline, svg::Polyline& tmp) {
    for (const auto& point: polyline.points()) {
        tmp.AddPoint({point.x(), point.y()});
    }
    AttrDeserialization(polyline, tmp);
}

void ColorDeserialization(svg::Color* underlayer_color, const serialization_data::Color* serializ_color) {
    if (serializ_color->has_color_()) {
        *underlayer_color = serializ_color->color();
    }
    if (serializ_color->has_rgb_()) {
        svg::Rgb tmp(serializ_color->rgb().red(),
                     serializ_color->rgb().green(),
                     serializ_color->rgb().blue());
        *underlayer_color = tmp;
    }
    if (serializ_color->has_rgba_()) {
        svg::Rgba tmp(serializ_color->rgba().red(),
                      serializ_color->rgba().green(),
                      serializ_color->rgba().blue(),
                      serializ_color->rgba().opacity());
        *underlayer_color = tmp;
    }
}

void RouteDeserialization(RequestHandler& requestHandler, const serialization_data::Bus& route) {
    domain::Bus current_route;
    current_route.is_roundtrip = route.is_round();
    current_route.stops_count = route.stops_count();
    current_route.unique_stops_count_ = route.unique_count();
    current_route.real_distance_ = route.real_distance();
    current_route.length_ = route.length();
    current_route.number_ = route.number();
    current_route.last_stop_.lat = route.last_stop().lat();
    current_route.last_stop_.lng = route.last_stop().lng();
    for (const auto& stop: route.stops()) {
        current_route.stops_.push_back(requestHandler.GetStopName(stop));
    }
    requestHandler.AddRouteToDatabaseFromSerializeData(current_route);
}

void StopDeserialization(RequestHandler& requestHandler, const serialization_data::Stop& stop) {
    domain::Stop current_stop;
    current_stop.name_ = stop.name();
    current_stop.coordinates_.lat = stop.coordinate().lat();
    current_stop.coordinates_.lng = stop.coordinate().lng();
    requestHandler.AddStopsToDatabase(current_stop);
}
