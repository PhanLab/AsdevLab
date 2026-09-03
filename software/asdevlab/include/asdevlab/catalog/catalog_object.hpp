#pragma once

#include <cctype>
#include <string>
#include <vector>

namespace asdevlab {
namespace catalog {

struct CatalogObject {
    std::string id;
    std::string name;
    std::string display_name;
    std::string type;
    std::string provider;
    double magnitude = 0.0;
    double ra = 0.0;
    double dec = 0.0;
    std::string epoch = "J2000";
    std::string coordinate_source = "catalog";
    std::string ephemeris_id;
    std::string messier;
    std::string ngc;
    std::string ic;
    std::vector<std::string> alias;
    std::string constellation;
    std::string fun_fact;

    void normalize() {
        if (display_name.empty()) {
            display_name = name;
        }

        if (epoch.empty()) {
            epoch = "J2000";
        }

        if (coordinate_source.empty()) {
            coordinate_source = inferCoordinateSource();
        }
    }

    bool requiresCatalogCoordinates() const {
        const auto lower_type = normalizeToken(type);
        const auto lower_provider = normalizeToken(provider);

        if (lower_provider == "ephemeris" || lower_provider == "tle") {
            return false;
        }

        if (lower_type == "planet" || lower_type == "moon" || lower_type == "sun" || lower_type == "comet" || lower_type == "asteroid" || lower_type == "satellite") {
            return false;
        }

        return true;
    }

    bool isValidForResolution() const {
        CatalogObject normalized = *this;
        normalized.normalize();

        if (!normalized.requiresCatalogCoordinates()) {
            return true;
        }

        return normalized.coordinate_source == "catalog" &&
               normalized.epoch == "J2000" &&
               normalized.ra != 0.0 &&
               normalized.dec != 0.0;
    }

private:
    static std::string normalizeToken(const std::string& value) {
        std::string normalized;
        normalized.reserve(value.size());
        for (char ch : value) {
            if (std::isalnum(static_cast<unsigned char>(ch))) {
                normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
            }
        }
        return normalized;
    }

    std::string inferCoordinateSource() const {
        const auto lower_provider = normalizeToken(provider);
        if (lower_provider == "ephemeris") {
            return "ephemeris";
        }
        if (lower_provider == "tle") {
            return "tle";
        }

        const auto lower_type = normalizeToken(type);
        if (lower_type == "planet" || lower_type == "moon" || lower_type == "sun" || lower_type == "comet" || lower_type == "asteroid" || lower_type == "satellite") {
            return "ephemeris";
        }

        return "catalog";
    }
};

} // namespace catalog
} // namespace asdevlab
