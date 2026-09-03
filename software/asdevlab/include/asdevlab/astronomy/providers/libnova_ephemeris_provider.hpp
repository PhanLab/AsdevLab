#pragma once

#include "asdevlab/astronomy/ephemeris_provider.hpp"
#include "asdevlab/catalog/catalog_object.hpp"
#include "asdevlab/catalog/coordinate_provider.hpp"

namespace asdevlab {
namespace astronomy {

class LibnovaEphemerisProvider : public EphemerisProvider, public catalog::CoordinateProvider {
public:
    bool supports(const catalog::CatalogObject& object) const override;
    catalog::ResolvedTarget resolve(const catalog::CatalogObject& object, const catalog::ObservationContext& context) const override;
    catalog::ResolvedTarget resolve(const std::string& target_name, const catalog::ObservationContext& context) const override;
};

} // namespace astronomy
} // namespace asdevlab
