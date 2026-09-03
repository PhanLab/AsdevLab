#pragma once

#include "asdevlab/catalog/coordinate_provider.hpp"
#include "asdevlab/catalog/resolved_target.hpp"

#include <cctype>
#include <string>

namespace asdevlab {
namespace catalog {

class SatelliteCoordinateProvider : public CoordinateProvider {
public:
    bool supports(const CatalogObject& object) const override {
        const auto lower_provider = normalizeType(object.provider);
        return lower_provider == "satellite" || normalizeType(object.type) == "satellite";
    }

    ResolvedTarget resolve(const CatalogObject& object, const ObservationContext& context) const override {
        ResolvedTarget target;
        target.epoch = object.epoch.empty() ? "J2000" : object.epoch;
        target.ra_hours = 0.0;
        target.dec_degrees = 0.0;
        target.alt_degrees = 0.0;
        target.az_degrees = 0.0;
        target.distance = 0.0;
        target.rise = "unknown";
        target.transit = "unknown";
        target.set = "unknown";
        target.visibility = false;
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
