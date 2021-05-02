#pragma once

#include <transport_catalogue.pb.h>
#include <graph.pb.h>
#include <transport_router.pb.h>

#include "request_handler.h"
#include "transport_router.h"

void Serialization(std::ostream& out, const RequestHandler& requestHandler, const router::TransportRouter& router);

void Deserialization(std::istream& in, RequestHandler& requestHandler, router::TransportRouter& router);

void RouteSerialization(serialization_data::TransportCatalogue& data,
                        const RequestHandler& requestHandler);

void StopSerialization(serialization_data::TransportCatalogue& data, const domain::Stop& stop);

void DistanceSerialization(serialization_data::TransportCatalogue& data,
                           const RequestHandler& requestHandler);

void StopDeserialization(RequestHandler& requestHandler, const serialization_data::Stop& stop);

void RouteDeserialization(RequestHandler& requestHandler, const serialization_data::Bus& route);

void SerializationColor(serialization_data::Color* data, const svg::Color& color);

void
PolylineSerialization(serialization_data::TransportCatalogue& data, const std::shared_ptr<svg::Polyline>& polyline);

void TextPolylineSerialization(serialization_data::TransportCatalogue& data, const std::shared_ptr<svg::Text>& text);

void TextCircleSerialization(serialization_data::TransportCatalogue& data, const std::shared_ptr<svg::Text>& text);

void CircleSerialization(serialization_data::TransportCatalogue& data, const std::shared_ptr<svg::Circle>& circle);

void MapSerialization(const RequestHandler& requestHandler, serialization_data::TransportCatalogue& data);

void ColorDeserialization(svg::Color* underlayer_color, const serialization_data::Color* serializ_color);

void PolylineDeserialization(const serialization_data::Polyline& polyline, svg::Polyline& tmp);

void TextDeserialization(const serialization_data::Text& text, svg::Text& tmp);

void CircleDeserialization(const serialization_data::Circle& circle, svg::Circle& tmp);

void MapDeserialization(const serialization_data::TransportCatalogue& data, Render& map);

void GraphSerialization(const router::TransportRouter& router, serialization_data::TransportCatalogue& data);

void GraphDeserialization(router::TransportRouter& router, const serialization_data::TransportCatalogue& data);

void InternalDataSerialization(const router::TransportRouter& router, serialization_data::TransportCatalogue& data);

void InternalDataDeserialization(router::TransportRouter& router, const serialization_data::TransportCatalogue& data);

void ActivitySerialization(const RequestHandler& requestHandler, const router::TransportRouter& router,
                           serialization_data::TransportCatalogue& data);

void ActivityDeserialization(const RequestHandler& requestHandler, router::TransportRouter& router,
                             const serialization_data::TransportCatalogue& data);


template <typename Current, typename Serialization>
void AttrSerialization(const Current& object, Serialization* current_object) {
    if (object->getFillColor().has_value()) {
        SerializationColor(current_object->mutable_attribute()->mutable_fill_color(), object->getFillColor().value());
        current_object->mutable_attribute()->set_has_fill_color_(true);
    }
    if (object->getStrokeColor().has_value()) {
        SerializationColor(current_object->mutable_attribute()->mutable_stroke_color(),
                           object->getStrokeColor().value());
        current_object->mutable_attribute()->set_has_stroke_color_(true);
    }
    if (object->getStrokeWidth().has_value()) {
        current_object->mutable_attribute()->set_stroke_with(object->getStrokeWidth().value());
        current_object->mutable_attribute()->set_has_stroke_with_(true);
    }
    if (object->getStrokeLinecap().has_value()) {
        current_object->mutable_attribute()->set_stroke_linecap(
                static_cast<serialization_data::StrokeLineCap>(static_cast<int>(object->getStrokeLinecap().value())));
        current_object->mutable_attribute()->set_has_stroke_linecap_(true);

    }
    if (object->getStrokeLinejoin().has_value()) {
        current_object->mutable_attribute()->set_stroke_linejoin(
                static_cast<serialization_data::StrokeLineJoin>(static_cast<int>(object->getStrokeLinejoin().value())));
        current_object->mutable_attribute()->set_has_stroke_linejoin_(true);
    }
}

template <typename Serialization, typename Current>
void AttrDeserialization(const Serialization& object, Current& tmp) {
    if (object.attribute().has_fill_color_()) {
        svg::Color color;
        ColorDeserialization(&color, &object.attribute().fill_color());
        tmp.SetFillColor(color);
    }
    if (object.attribute().has_stroke_color_()) {
        svg::Color color;
        ColorDeserialization(&color, &object.attribute().stroke_color());
        tmp.SetStrokeColor(color);
    }
    if (object.attribute().has_stroke_with_()) {
        tmp.SetStrokeWidth(object.attribute().stroke_with());
    }
    if (object.attribute().has_stroke_linecap_()) {
        tmp.SetStrokeLineCap(static_cast<svg::StrokeLineCap>(static_cast<int>(object.attribute().stroke_linecap())));
    }
    if (object.attribute().has_stroke_linejoin_()) {
        tmp.SetStrokeLineJoin(static_cast<svg::StrokeLineJoin>(static_cast<int>(object.attribute().stroke_linejoin())));
    }
}