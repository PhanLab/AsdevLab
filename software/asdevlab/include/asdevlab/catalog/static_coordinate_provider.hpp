#pragma once

#include "asdevlab/catalog/coordinate_provider.hpp"

namespace asdevlab {
namespace catalog {

class StaticCoordinateProvider : public CoordinateProvider {
public:
    bool supports(const CatalogObject& object) const override {
        CatalogObject normalized = object;
        normalized.normalize();

        const auto lower_provider = normalizeType(normalized.provider);
        const auto lower_type = normalizeType(normalized.type);

        if (!lower_provider.empty() && lower_provider != "static") {
            return false;
        }

        if (lower_type == "sun" || lower_type == "moon" || lower_type == "mercury" || lower_type == "venus" || lower_type == "mars" || lower_type == "jupiter" || lower_type == "saturn" || lower_type == "uranus" || lower_type == "neptune" || lower_type == "satellite" || lower_type == "asteroid" || lower_type == "comet") {
            return false;
        }

        return normalized.coordinate_source == "catalog" || normalized.coordinate_source.empty();
    }

    ResolvedTarget resolve(const CatalogObject& object, const ObservationContext& context) const override {
        CatalogObject normalized = object;
        normalized.normalize();
        if (!supports(normalized)) {
            return {};
        }

        if (normalized.ra == 0.0 && normalized.dec == 0.0 && normalized.coordinate_source == "catalog") {
            return {};
        }

        auto target = buildTarget(normalized.ra, normalized.dec, context, normalized.epoch, 0.0, true);
        target.resolved = true;
        return target;
    }

    bool resolve(const CatalogObject& object, double& ra, double& dec) const override {
        CatalogObject normalized = object;
        normalized.normalize();
        ra = normalized.ra;
        dec = normalized.dec;
        return (ra != 0.0) || (dec != 0.0);
    }
};

} // namespace catalog
} // namespace asdevlab
