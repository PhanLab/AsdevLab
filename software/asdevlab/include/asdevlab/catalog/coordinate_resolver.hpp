#pragma once

#include "asdevlab/catalog/catalog_object.hpp"
#include "asdevlab/astronomy/providers/libnova_ephemeris_provider.hpp"
#include "asdevlab/catalog/coordinate_provider.hpp"
#include "asdevlab/catalog/equatorial_coordinate.hpp"
#include "asdevlab/catalog/observation_context.hpp"
#include "asdevlab/catalog/resolved_target.hpp"
#include "asdevlab/catalog/static_coordinate_provider.hpp"

#include <cctype>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace asdevlab {
namespace catalog {

class CoordinateResolver {
public:
    CoordinateResolver() {
        registerProvider(std::make_shared<StaticCoordinateProvider>());
        registerProvider(std::make_shared<astronomy::LibnovaEphemerisProvider>());
    }

    explicit CoordinateResolver(std::shared_ptr<CoordinateProvider> provider) : CoordinateResolver() {
        if (provider) {
            providers_.insert(providers_.begin(), std::move(provider));
        }
    }

    void registerProvider(std::shared_ptr<CoordinateProvider> provider) {
        if (provider) {
            providers_.push_back(std::move(provider));
        }
    }

    std::shared_ptr<CoordinateProvider> createProvider(const CatalogObject& object) const {
        if (object.coordinate_source == "ephemeris") {
            return std::make_shared<astronomy::LibnovaEphemerisProvider>();
        }
        if (object.coordinate_source == "tle") {
            return nullptr;
        }
        return std::make_shared<StaticCoordinateProvider>();
    }

    static std::string normalizeToken(const std::string& token) {
        std::string normalized;
        normalized.reserve(token.size());
        for (char ch : token) {
            if (std::isalnum(static_cast<unsigned char>(ch))) {
                normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
            }
        }
        return normalized;
    }

    ResolvedTarget resolveTarget(const CatalogObject& object, const ObservationContext& context) const {
        if (object.coordinate_source == "tle") {
            return {};
        }

        for (size_t i = 0; i < providers_.size(); ++i) {
            const auto& provider = providers_[i];
            try {
                if (!provider->supports(object)) {
                    continue;
                }
                return provider->resolve(object, context);
            } catch (const std::exception& ex) {
                std::cerr << "CoordinateResolver: provider[" << i << "] supports() threw for object id='" << object.id << "': " << ex.what() << "\n";
                throw;
            } catch (...) {
                std::cerr << "CoordinateResolver: provider[" << i << "] supports() threw unknown exception for object id='" << object.id << "'\n";
                throw;
            }
        }

        const auto fallback_provider = createProvider(object);
        if (fallback_provider && fallback_provider->supports(object)) {
            return fallback_provider->resolve(object, context);
        }

        return {};
    }

    EquatorialCoordinate resolve(const CatalogObject& object, const ObservationContext& context) const {
        const auto target = resolveTarget(object, context);
        return {target.ra_hours, target.dec_degrees};
    }

private:
    std::vector<std::shared_ptr<CoordinateProvider>> providers_;
};

} // namespace catalog
} // namespace asdevlab
