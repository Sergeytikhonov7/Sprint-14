#pragma once

#include <deque>
#include <string>
#include <vector>
#include <set>
#include <memory>

#include "geo.h"
#include "svg.h"

namespace domain {

    struct Stop {
        std::string name_;
        geo::Coordinates coordinates_;
    };

    struct StopComp {
        bool operator()(const Stop* lhs, const Stop* rhs) const {
            return lhs->name_ < rhs->name_;
        }
    };

    struct Bus {
        bool is_roundtrip = false;
        size_t stops_count = 0;
        size_t unique_stops_count = 0;
        unsigned int real_distance = 0;
        double length = 0;
        std::string number;
        geo::Coordinates last_stop;
        std::deque<const Stop*> stops;
    };
}