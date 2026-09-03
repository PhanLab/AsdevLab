#include "asdevlab/astronomy/astronomy_service.hpp"
#include "asdevlab/astronomy/astronomy_utils.hpp"

namespace asdevlab {
namespace astronomy {

AstronomyService::AstronomyService(std::shared_ptr<EphemerisProvider> provider) : provider_(std::move(provider)) {}

catalog::ResolvedTarget AstronomyService::resolve(const std::string& target_name, const catalog::ObservationContext& context) const {
    if (!provider_) {
        return {};
    }
    return provider_->resolve(target_name, context);
}

catalog::ResolvedTarget AstronomyService::computeHorizontalCoordinates(const catalog::ResolvedTarget& target, const catalog::ObservationContext& context) const {
    if (!target.resolved || target.horizontal_computed) {
        return target;
    }

    catalog::ResolvedTarget updated = target;
    const auto horizontal = compute_horizontal_coordinates(updated.ra_hours, updated.dec_degrees, context);
    updated.alt_degrees = horizontal.alt_degrees;
    updated.az_degrees = horizontal.az_degrees;
    updated.horizontal_computed = true;
    updated.visibility = updated.alt_degrees > 0.0;
    return updated;
}

} // namespace astronomy
} // namespace asdevlab
