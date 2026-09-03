#pragma once

#include <string>

namespace asdevlab {
namespace catalog {

struct ResolvedTarget {
    double ra_hours = 0.0;
    double dec_degrees = 0.0;
    double alt_degrees = 0.0;
    double az_degrees = 0.0;
    double distance = 0.0;
    std::string rise;
    std::string transit;
    std::string set;
    bool visibility = false;
    std::string epoch;
    bool resolved = false;
    bool horizontal_computed = false;
};

} // namespace catalog
} // namespace asdevlab
