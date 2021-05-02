#pragma once

#include <sstream>
#include <algorithm>

#include "geo.h"
#include "svg.h"
#include "domain.h"

struct Render {
    unsigned int bus_label_font_size_ = 0;
    unsigned int stop_label_font_size_ = 0;
    double width_ = 0;
    double height_ = 0;
    double padding_ = 0;
    double line_width_ = 0;
    double stop_radius_ = 0;
    double underlayer_width_ = 0;
    double min_lat_ = 1000000;
    double min_lon_ = 1000000;
    double max_lat_ = -1000000;
    double max_lon_ = -1000000;
    double zoom_coef_ = 0;
    svg::Point bus_label_offset_;
    svg::Point stop_label_offset_;
    svg::Color underlayer_color_;
    std::vector<svg::Color> color_palette_;
    std::vector<std::shared_ptr<svg::Polyline>> polyline_;
    std::vector<std::shared_ptr<svg::Text>> text_polyline_;
    std::vector<std::shared_ptr<svg::Circle>> circle_;
    std::vector<std::shared_ptr<svg::Text>> text_circle_;
};

namespace renderer {
    class MapRenderer {
    public:
        MapRenderer() = default;

        Render& GetRender();

        void CalculationCoef();

        svg::Point CalculateCoord(const geo::Coordinates& point);

        void AddRouteToMap(const domain::Bus* pBus);

        void AddStopToMap(const std::set<const domain::Stop*, domain::StopComp>& uniq_stops);

    private:
        Render render_;
        int current_color_palette_ = 0;
    };

}