#pragma once

#include "asdevlab/catalog/observation_context.hpp"
#include "asdevlab/catalog/resolved_target.hpp"

#include <string>

namespace asdevlab {
namespace astronomy {

class EphemerisProvider {
public:
    virtual ~EphemerisProvider() = default;
    virtual catalog::ResolvedTarget resolve(const std::string& target_name, const catalog::ObservationContext& context) const = 0;
};

} // namespace astronomy
} // namespace asdevlab
