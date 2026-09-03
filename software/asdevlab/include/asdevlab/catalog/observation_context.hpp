#pragma once

#include <chrono>
#include <string>

namespace asdevlab {
namespace catalog {

struct ObservationContext {
    std::chrono::system_clock::time_point utc_time;
    double latitude_degrees = 0.0;
    double longitude_degrees = 0.0;
    double elevation_meters = 0.0;
    double pressure_mbar = 0.0;
    double temperature_c = 0.0;
    std::string timezone;
};

} // namespace catalog
} // namespace asdevlab
