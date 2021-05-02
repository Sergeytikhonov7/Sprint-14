#include "map_renderer.h"

namespace renderer {

    Render& MapRenderer::GetRender() {
        return render_;
    }

    void MapRenderer::CalculationCoef() {
        if (((render_.max_lon_ - render_.min_lon_) == 0) &&
            ((render_.max_lat_ - render_.min_lat_) == 0)) {
            render_.zoom_coef_ = 0;
        } else if ((render_.max_lon_ - render_.min_lon_) == 0) {
            render_.zoom_coef_ = (render_.height_ - 2 * render_.padding_) /
                                 (render_.max_lat_ - render_.min_lat_);
        } else if ((render_.max_lat_ - render_.min_lat_) == 0) {
            render_.zoom_coef_ = (render_.width_ - 2 * render_.padding_) /
                                 (render_.max_lon_ - render_.min_lon_);
        } else {
            render_.zoom_coef_ = std::min(
                    ((render_.height_ - 2 * render_.padding_) / // width_zoom_coef
                     (render_.max_lat_ - render_.min_lat_)),
                    ((render_.width_ - 2 * render_.padding_) / // height_zoom_coef
                     (render_.max_lon_ - render_.min_lon_)));
        }
    }

    svg::Point MapRenderer::CalculateCoord(const geo::Coordinates& point) {
        double x = (point.lng - render_.min_lon_) * render_.zoom_coef_ + render_.padding_;
        double y = (render_.max_lat_ - point.lat) * render_.zoom_coef_ + render_.padding_;
        return {x, y};
    }

    void MapRenderer::AddRouteToMap(const domain::Bus* pBus) {
        using namespace std::literals;
        svg::Polyline polyline;
        for (const auto& stops : pBus->stops) {
            polyline.AddPoint(CalculateCoord(stops->coordinates_));
        }
        size_t count_color = current_color_palette_;
        polyline.SetStrokeColor(render_.color_palette_[count_color]).SetFillColor(svg::NoneColor).SetStrokeWidth(
                render_.line_width_).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(
                svg::StrokeLineJoin::ROUND);
        render_.polyline_.emplace_back(std::make_shared<svg::Polyline>(polyline));
        svg::Text inscription;
        inscription.SetPosition(CalculateCoord(pBus->stops.begin().operator*()->coordinates_)).
                SetOffset(render_.bus_label_offset_).
                SetFontSize(render_.bus_label_font_size_).
                SetFontFamily("Verdana"s).
                SetFontWeight("bold"s).
                SetData(std::string(pBus->number));
        svg::Text substrate(inscription);
        substrate.SetFillColor(render_.underlayer_color_).
                SetStrokeColor(render_.underlayer_color_).
                SetStrokeWidth(render_.underlayer_width_).
                SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        inscription.SetFillColor(render_.color_palette_[count_color]);
        render_.text_polyline_.emplace_back(std::make_shared<svg::Text>(substrate));
        render_.text_polyline_.emplace_back(std::make_shared<svg::Text>(inscription));
        if (!pBus->is_roundtrip) {
            if (CalculateCoord(pBus->stops.begin().operator*()->coordinates_).x != CalculateCoord(pBus->last_stop).x &&
                CalculateCoord(pBus->stops.begin().operator*()->coordinates_).y != CalculateCoord(pBus->last_stop).y) {
                svg::Text inscription2(inscription);
                inscription2.SetPosition(CalculateCoord(pBus->last_stop));
                svg::Text substrate2(substrate);
                substrate2.SetPosition(CalculateCoord(pBus->last_stop));
                render_.text_polyline_.emplace_back(std::make_shared<svg::Text>(substrate2));
                render_.text_polyline_.emplace_back(std::make_shared<svg::Text>(inscription2));
            }
        }
        count_color >= render_.color_palette_.size() - 1 ? count_color = 0 : ++count_color;
        current_color_palette_ = count_color;
    }

    void MapRenderer::AddStopToMap(const std::set<const domain::Stop*, domain::StopComp>& uniq_stops) {
        using namespace std::literals;
        for (auto& uniq_stop : uniq_stops) {
            svg::Circle circle;
            circle.SetCenter(CalculateCoord(uniq_stop->coordinates_));
            circle.SetRadius(render_.stop_radius_);
            circle.SetFillColor("white"s);
            render_.circle_.emplace_back(std::make_shared<svg::Circle>(circle));
            svg::Text stop_names;
            stop_names.SetPosition(CalculateCoord(uniq_stop->coordinates_)).
                    SetOffset(render_.stop_label_offset_).
                    SetFontSize(render_.stop_label_font_size_).
                    SetFontFamily("Verdana"s).
                    SetData(std::move(uniq_stop->name_));
            svg::Text substrate(stop_names);
            substrate.SetFillColor(render_.underlayer_color_).
                    SetStrokeColor(render_.underlayer_color_).
                    SetStrokeWidth(render_.underlayer_width_).
                    SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                    SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            stop_names.SetFillColor("black"s);
            render_.text_circle_.emplace_back(std::make_shared<svg::Text>(substrate));
            render_.text_circle_.emplace_back(std::make_shared<svg::Text>(stop_names));
        }
    }
}
