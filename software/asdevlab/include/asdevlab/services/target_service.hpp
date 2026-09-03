#pragma once

#include "asdevlab/catalog/catalog_engine.hpp"
#include "asdevlab/catalog/coordinate_resolver.hpp"
#include "asdevlab/catalog/observation_context.hpp"
#include "asdevlab/services/motion_service.hpp"

#include <string>
#include <vector>

namespace asdevlab {

class TargetService {
public:
    struct SearchResult {
        std::vector<catalog::CatalogObject> results;
    };

    struct ResolveResult {
        bool ok = false;
        std::string message;
        catalog::CatalogObject object;
        catalog::ResolvedTarget target;
    };

    TargetService(catalog::CatalogEngine& catalog, const catalog::CoordinateResolver& resolver, MotionService& motion);

    SearchResult search(const std::string& keyword) const;
    const catalog::CatalogObject* getObject(const std::string& id) const;
    ResolveResult resolveTarget(const std::string& id, const catalog::ObservationContext& context) const;

    MotionCommandResult gotoObject(const std::string& id, const catalog::ObservationContext& context);
    MotionCommandResult syncObject(const std::string& id, const catalog::ObservationContext& context);
    MotionCommandResult previewObject(const std::string& id, const catalog::ObservationContext& context);

private:
    catalog::CatalogEngine& catalog_;
    const catalog::CoordinateResolver& resolver_;
    MotionService& motion_;
};

} // namespace asdevlab
