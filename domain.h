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
        size_t unique_stops_count_ = 0;
        unsigned int real_distance_ = 0;
        double length_ = 0;
        std::string number_;
        geo::Coordinates last_stop_;
        std::deque<const Stop*> stops_;
    };
}