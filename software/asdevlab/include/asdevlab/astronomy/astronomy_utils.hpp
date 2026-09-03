#pragma once

#include "asdevlab/catalog/observation_context.hpp"

namespace asdevlab {
namespace catalog {
struct ResolvedTarget;
}
namespace astronomy {

struct HorizontalCoordinates {
    double alt_degrees = 0.0;
    double az_degrees = 0.0;
};

HorizontalCoordinates compute_horizontal_coordinates(double ra_hours, double dec_degrees, const catalog::ObservationContext& context);

double compute_altitude_degrees(double ra_hours, double dec_degrees, const catalog::ObservationContext& context);

// Overload that accepts raw site/time parameters so callers don't need to
// construct an ObservationContext when only altitude is required.
double compute_altitude_degrees(double ra_hours, double dec_degrees, double latitude_degrees, double longitude_degrees, std::time_t utc, double pressure_mbar = 0.0, double temperature_c = 0.0);

} // namespace astronomy
} // namespace asdevlab
