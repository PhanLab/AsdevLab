#pragma once

#include "asdevlab/astronomy/ephemeris_provider.hpp"
#include "asdevlab/catalog/observation_context.hpp"
#include "asdevlab/catalog/resolved_target.hpp"

#include <memory>
#include <string>

namespace asdevlab {
namespace astronomy {

class AstronomyService {
public:
    explicit AstronomyService(std::shared_ptr<EphemerisProvider> provider);

    catalog::ResolvedTarget resolve(const std::string& target_name, const catalog::ObservationContext& context) const;
    catalog::ResolvedTarget computeHorizontalCoordinates(const catalog::ResolvedTarget& target, const catalog::ObservationContext& context) const;

private:
    std::shared_ptr<EphemerisProvider> provider_;
};

} // namespace astronomy
} // namespace asdevlab
