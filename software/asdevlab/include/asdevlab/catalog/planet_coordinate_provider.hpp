#pragma once

#include "asdevlab/catalog/coordinate_provider.hpp"
#include "asdevlab/catalog/resolved_target.hpp"

#include <cctype>
#include <string>

namespace asdevlab {
namespace catalog {

class PlanetCoordinateProvider : public CoordinateProvider {
public:
    bool supports(const CatalogObject& object) const override {
        const auto lower_provider = normalizeType(object.provider);
        const auto lower = normalizeType(object.type);
        return lower_provider == "planet" || lower_provider == "ephemeris" || lower_provider == "sun" || lower_provider == "moon" || lower == "sun" || lower == "moon" || lower == "mercury" || lower == "venus" || lower == "mars" || lower == "jupiter" || lower == "saturn" || lower == "uranus" || lower == "neptune";
    }

    ResolvedTarget resolve(const CatalogObject& object, const ObservationContext& context) const override {
        ResolvedTarget target;
        const auto lower = normalizeType(object.type);
        target.epoch = object.epoch.empty() ? "J2000" : object.epoch;
        target.visibility = true;
        target.alt_degrees = 0.0;
        target.az_degrees = 0.0;
        target.distance = 0.0;
        target.rise = "unknown";
        target.transit = "unknown";
        target.set = "unknown";

        if (lower == "sun") {
            target.ra_hours = 0.0;
            target.dec_degrees = 0.0;
        } else if (lower == "moon") {
            target.ra_hours = 0.0;
            target.dec_degrees = 0.0;
        } else if (lower == "mercury" || lower == "venus" || lower == "mars" || lower == "jupiter" || lower == "saturn" || lower == "uranus" || lower == "neptune") {
            target.ra_hours = 0.0;
            target.dec_degrees = 0.0;
        } else {
            target.visibility = false;
        }

        (void)context;
        return target;
    }

private:
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
};

} // namespace catalog
} // namespace asdevlab
