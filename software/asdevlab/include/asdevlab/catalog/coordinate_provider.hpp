#pragma once

#include "asdevlab/catalog/catalog_object.hpp"
#include "asdevlab/catalog/equatorial_coordinate.hpp"
#include "asdevlab/catalog/observation_context.hpp"
#include "asdevlab/catalog/resolved_target.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cctype>
#include <string>

namespace asdevlab {
namespace catalog {

class CoordinateProvider {
public:
    virtual ~CoordinateProvider() = default;

    virtual bool supports(const CatalogObject& object) const = 0;
    virtual ResolvedTarget resolve(const CatalogObject& object, const ObservationContext& context) const = 0;

    virtual bool resolve(const CatalogObject& object, double& ra, double& dec) const {
        // Default: return stored equatorial coordinates without creating an ObservationContext.
        ra = object.ra;
        dec = object.dec;
        return (ra != 0.0) || (dec != 0.0);
    }

protected:
    static std::string normalizeType(const std::string& value) {
        std::string normalized;
        normalized.reserve(value.size());
        for (char ch : value) {
            if (std::isalnum(static_cast<unsigned char>(ch))) {
                normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
            }
        }
        return normalized;
    }

    static double degToRad(double degrees) {
        return degrees * 0.017453292519943295;
    }

    static double radToDeg(double radians) {
        return radians * 57.29577951308232;
    }

    static double clamp(double value, double min_value, double max_value) {
        return std::max(min_value, std::min(max_value, value));
    }

    static double julianDay(const ObservationContext& context) {
        using namespace std::chrono;
        auto timestamp = context.utc_time;
        if (timestamp == time_point<system_clock>{}) {
            timestamp = system_clock::now();
        }
        const auto seconds = duration_cast<std::chrono::seconds>(timestamp.time_since_epoch()).count();
        return 2440587.5 + static_cast<double>(seconds) / 86400.0;
    }

    static ResolvedTarget buildTarget(
        double ra_hours,
        double dec_degrees,
        const ObservationContext& context,
        const std::string& epoch,
        double distance,
        bool visibility,
        const std::string& rise = "unknown",
        const std::string& transit = "unknown",
        const std::string& set = "unknown"
    ) {
        ResolvedTarget target;
        target.ra_hours = ra_hours;
        target.dec_degrees = dec_degrees;
        target.distance = distance;
        target.epoch = epoch;
        target.visibility = visibility;
        target.rise = rise;
        target.transit = transit;
        target.set = set;

        // Alt/Az and rise/set calculations belong to the central AstronomyService (libnova).
        // Providers should not compute horizontal coordinates here to avoid duplication.

        return target;
    }
};

} // namespace catalog
} // namespace asdevlab
