#include "asdevlab/services/target_service.hpp"

#include <iostream>
#include <optional>
#include <sstream>

namespace asdevlab {

TargetService::TargetService(catalog::CatalogEngine& catalog, const catalog::CoordinateResolver& resolver, MotionService& motion)
    : catalog_(catalog)
    , resolver_(resolver)
    , motion_(motion) {}

namespace {

std::optional<catalog::CatalogObject> resolveCatalogObject(const catalog::CatalogEngine& catalog, const std::string& query) {
    auto exact = catalog.getObject(query);
    if (exact.has_value()) {
        return **exact;
    }

    const auto matches = catalog.search(query);
    if (!matches.empty()) {
        return matches.front();
    }
    return std::nullopt;
}

} // namespace

TargetService::SearchResult TargetService::search(const std::string& keyword) const {
    SearchResult r;
    auto list = catalog_.search(keyword);
    r.results.insert(r.results.end(), list.begin(), list.end());
    return r;
}

const catalog::CatalogObject* TargetService::getObject(const std::string& id) const {
    auto opt = catalog_.getObject(id);
    if (!opt.has_value()) {
        return nullptr;
    }
    return *opt;
}

TargetService::ResolveResult TargetService::resolveTarget(const std::string& id, const catalog::ObservationContext& context) const {
    ResolveResult result;
    const auto obj = resolveCatalogObject(catalog_, id);
    if (!obj.has_value()) {
        result.message = "object not found";
        return result;
    }

    result.object = *obj;
    result.target = resolver_.resolveTarget(result.object, context);
    result.ok = result.target.resolved;
    if (!result.ok) {
        result.message = "object could not be resolved";
    }
    return result;
}

MotionCommandResult TargetService::gotoObject(const std::string& id, const catalog::ObservationContext& context) {
    const auto obj = resolveCatalogObject(catalog_, id);
    MotionCommandResult result{false, ""};
    if (!obj.has_value()) {
        result.ok = false;
        result.log = "object not found";
        return result;
    }
    auto resolved = resolver_.resolveTarget(*obj, context);
    if (!resolved.resolved) {
        result.ok = false;
        result.log = "object could not be resolved";
        return result;
    }

    std::string log;
    auto goto_res = motion_.goto_target(resolved, log);
    result.ok = goto_res.ok;
    result.log = log;
    return result;
}

MotionCommandResult TargetService::syncObject(const std::string& id, const catalog::ObservationContext& context) {
    const auto obj = resolveCatalogObject(catalog_, id);
    MotionCommandResult result{false, ""};
    if (!obj.has_value()) {
        result.ok = false;
        result.log = "object not found";
        return result;
    }
    auto resolved = resolver_.resolveTarget(*obj, context);
    if (!resolved.resolved) {
        result.ok = false;
        result.log = "object could not be resolved";
        return result;
    }

    std::string log;
    auto res = motion_.sync(resolved, log);
    result.ok = res.ok;
    result.log = log;
    return result;
}

MotionCommandResult TargetService::previewObject(const std::string& id, const catalog::ObservationContext& context) {
    const auto obj = resolveCatalogObject(catalog_, id);
    MotionCommandResult result{false, ""};
    if (!obj.has_value()) {
        result.ok = false;
        result.log = "object not found";
        return result;
    }

    const auto resolved = resolver_.resolveTarget(*obj, context);
    if (!resolved.resolved) {
        result.ok = false;
        result.log = "object could not be resolved";
        return result;
    }

    std::ostringstream oss;
    oss << "ra=" << resolved.ra_hours << ", dec=" << resolved.dec_degrees;
    result.ok = true;
    result.log = oss.str();
    return result;
}

} // namespace asdevlab
