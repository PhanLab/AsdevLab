#pragma once

#include "asdevlab/catalog/coordinate_provider.hpp"

namespace asdevlab {
namespace catalog {

class EphemerisCoordinateProvider : public CoordinateProvider {
public:
    virtual ~EphemerisCoordinateProvider() override = default;

    bool supports(const CatalogObject& object) const override {
        (void)object;
        return false;
    }

    ResolvedTarget resolve(const CatalogObject& object, const ObservationContext& context) const override {
        return resolveTarget(object, context);
    }

    virtual EquatorialCoordinate getCoordinates(
        const CatalogObject& object,
        const ObservationContext& context
    ) const = 0;

    virtual ResolvedTarget resolveTarget(
        const CatalogObject& object,
        const ObservationContext& context
    ) const {
        const auto coordinates = getCoordinates(object, context);
        return buildTarget(coordinates.ra_hours, coordinates.dec_degrees, context, "J2000", 0.0, true);
    }
};

} // namespace catalog
} // namespace asdevlab
