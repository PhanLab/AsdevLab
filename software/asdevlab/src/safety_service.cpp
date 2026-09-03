#include "asdevlab/safety_service.hpp"

#include <algorithm>
#include <cmath>
#include <ctime>
#include "asdevlab/astronomy/astronomy_utils.hpp"

namespace asdevlab {

SafetyService::SafetyService()
    : last_reason_(),
      system_state_(SystemState::Normal) {}

namespace {

constexpr double kDegreesPerRadian = 180.0 / M_PI;
constexpr double kHoursPerRadian = 12.0 / M_PI;

// Hardcoded site location until a real config is available.
constexpr double kSiteLatitudeDegrees = 35.0;
constexpr double kSiteLongitudeDegrees = -105.0;
constexpr double kMinAltitudeDegrees = 10.0;
constexpr double kMaxAltitudeDegrees = 90.0;

std::time_t current_utc_time() {
    return std::time(nullptr);
}

// Altitude computation is delegated to the astronomy layer (libnova) to avoid
// duplicate astronomy math across components.
double altitude_degrees(const EquatorialCoord& target, double latitude_degrees, std::time_t utc) {
        // Compute altitude directly from site/time parameters without building an
        // ObservationContext since SafetyService shouldn't construct contexts.
        const std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        return astronomy::compute_altitude_degrees(target.ra_hours, target.dec_degrees, latitude_degrees, kSiteLongitudeDegrees, now, /* pressure */ 1013.25, /* temperature */ 20.0);
}

} // namespace

bool SafetyService::can_send_motion_command(const std::string& command,
                                            std::optional<EquatorialCoord> target) const {
    last_reason_.clear();
    if (command == "park" || command == "stop") {
        return true;
    }

    if (system_state_ == SystemState::Focusing && (command == "goto" || command == "move")) {
        last_reason_ = "blocked while focusing";
        return false;
    }

    if ((command == "goto" || command == "move") && target) {
        const double alt = altitude_degrees(*target, kSiteLatitudeDegrees, current_utc_time());
        if (alt < kMinAltitudeDegrees) {
            last_reason_ = "target below minimum altitude";
            return false;
        }
        if (alt > kMaxAltitudeDegrees) {
            last_reason_ = "target above maximum altitude";
            return false;
        }
    }

    return true;
}

std::string SafetyService::reason() const {
    return last_reason_;
}

void SafetyService::set_system_state(SystemState state) {
    system_state_ = state;
}

SafetyService::SystemState SafetyService::system_state() const {
    return system_state_;
}

} // namespace asdevlab
