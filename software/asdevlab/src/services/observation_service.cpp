#include "asdevlab/services/observation_service.hpp"

#include "asdevlab/astronomy/astronomy_service.hpp"
#include "asdevlab/services/motion_service.hpp"
#include "asdevlab/services/target_service.hpp"

#include <chrono>
#include <cstdlib>
#include <sstream>

namespace {

static std::string workflowStateToLegacyStage(asdevlab::ObservationService::WorkflowState state) {
    switch (state) {
        case asdevlab::ObservationService::WorkflowState::Searching: return "search";
        case asdevlab::ObservationService::WorkflowState::Resolving: return "resolve";
        case asdevlab::ObservationService::WorkflowState::Preview: return "preview";
        case asdevlab::ObservationService::WorkflowState::VisibilityCheck: return "visibility";
        case asdevlab::ObservationService::WorkflowState::Slewing: return "goto";
        case asdevlab::ObservationService::WorkflowState::Tracking: return "tracking";
        case asdevlab::ObservationService::WorkflowState::Completed: return "tracking";
        case asdevlab::ObservationService::WorkflowState::Aborted: return "abort";
        case asdevlab::ObservationService::WorkflowState::Error: return "error";
        case asdevlab::ObservationService::WorkflowState::Idle:
        default:
            return "idle";
    }
}

static std::string getEnv(const char* name, const std::string& fallback = "") {
    if (const char* value = std::getenv(name)) {
        return std::string(value);
    }
    return fallback;
}

static double getEnvDouble(const char* name, double fallback = 0.0) {
    const auto value = getEnv(name);
    if (value.empty()) {
        return fallback;
    }
    try {
        return std::stod(value);
    } catch (...) {
        return fallback;
    }
}

} // namespace

namespace asdevlab {

void ObservationService::setTargetService(TargetService& service) {
    target_service_ = &service;
}

void ObservationService::setMotionService(MotionService& service) {
    motion_service_ = &service;
}

void ObservationService::setAstronomyService(astronomy::AstronomyService& service) {
    astronomy_service_ = &service;
}

void ObservationService::setCurrentTarget(const std::string& target_id) {
    current_target_id_ = target_id;
}

void ObservationService::clearCurrentTarget() {
    current_target_id_.clear();
}

std::string ObservationService::currentTargetId() const {
    return current_target_id_;
}

catalog::ObservationContext ObservationService::makeObservationContext() const {
    catalog::ObservationContext context;
    context.utc_time = std::chrono::system_clock::now();
    context.latitude_degrees = getEnvDouble("ASDEVLAB_LATITUDE", 0.0);
    context.longitude_degrees = getEnvDouble("ASDEVLAB_LONGITUDE", 0.0);
    context.elevation_meters = getEnvDouble("ASDEVLAB_ELEVATION", 0.0);
    context.pressure_mbar = getEnvDouble("ASDEVLAB_PRESSURE", 0.0);
    context.temperature_c = getEnvDouble("ASDEVLAB_TEMPERATURE", 0.0);
    context.timezone = getEnv("ASDEVLAB_TIMEZONE", "UTC");
    return context;
}

ObservationService::TargetWorkflowStatus ObservationService::runTargetWorkflow(const std::string& target_query,
                                                                              const catalog::ObservationContext& context,
                                                                              bool require_visibility) {
    // Workflow orchestration: resolve the target, evaluate visibility, dispatch motion,
    // and report progress through a single status object.
    // This method currently assumes it is the only active workflow and mutates shared state
    // without synchronization. TODO: If concurrent workflows become possible, guard the
    // shared workflow/status state and current-target updates here.
    TargetWorkflowStatus status;
    status.state = WorkflowState::Searching;
    status.progress = 0.1;
    status.stage = workflowStateToLegacyStage(status.state);
    status.steps.push_back({"search", true, "catalog search started"});

    if (!target_service_ || !motion_service_) {
        status.state = WorkflowState::Error;
        status.ok = false;
        status.message = "observation workflow dependencies are not configured";
        status.progress = 0.0;
        status.steps.push_back({"dependencies", false, status.message});
        return status;
    }

    const auto search_result = target_service_->search(target_query);
    if (search_result.results.empty()) {
        status.state = WorkflowState::Error;
        status.ok = false;
        status.message = "no catalog objects matched the search query";
        status.progress = 0.0;
        status.stage = workflowStateToLegacyStage(status.state);
        status.steps.push_back({"search", false, status.message});
        return status;
    }

    status.catalog_object = search_result.results.front();
    status.steps.push_back({"search", true, "catalog object selected"});

    status.state = WorkflowState::Resolving;
    status.progress = 0.2;
    const auto resolve_result = target_service_->resolveTarget(target_query, context);
    if (!resolve_result.ok || !resolve_result.target.resolved) {
        status.state = WorkflowState::Error;
        status.ok = false;
        status.message = resolve_result.message.empty() ? "target could not be resolved" : resolve_result.message;
        status.progress = 0.0;
        status.stage = workflowStateToLegacyStage(status.state);
        status.steps.push_back({"resolve", false, status.message});
        return status;
    }

    status.resolved_target = resolve_result.target;
    setCurrentTarget(resolve_result.object.id);
    status.stage = workflowStateToLegacyStage(status.state);
    status.steps.push_back({"resolve", true, "target resolved"});

    if (astronomy_service_) {
        status.resolved_target = astronomy_service_->computeHorizontalCoordinates(status.resolved_target, context);
    }

    status.state = WorkflowState::Preview;
    status.progress = 0.4;
    status.steps.push_back({"preview", true, "target preview prepared"});

    const bool visible = status.resolved_target.visibility || !require_visibility;
    if (require_visibility && !visible) {
        status.state = WorkflowState::Error;
        status.ok = false;
        status.message = "target is not visible from the current site";
        status.progress = 0.0;
        status.stage = workflowStateToLegacyStage(status.state);
        status.steps.push_back({"visibility", false, status.message});
        return status;
    }

    status.state = WorkflowState::VisibilityCheck;
    status.progress = 0.6;
    status.steps.push_back({"visibility", true, visible ? "target is visible" : "visibility check skipped"});

    std::string log;
    const auto goto_result = motion_service_->goto_target(status.resolved_target, log);
    status.state = goto_result.ok ? WorkflowState::Slewing : WorkflowState::Error;
    status.progress = goto_result.ok ? 0.8 : 0.0;
    status.message = log;
    status.stage = workflowStateToLegacyStage(status.state);
    status.steps.push_back({"goto", goto_result.ok, log});

    if (!goto_result.ok) {
        status.ok = false;
        return status;
    }

    status.state = WorkflowState::Tracking;
    status.progress = 0.9;
    status.monitoring_message = "monitoring target state";
    status.steps.push_back({"monitor", true, status.monitoring_message});

    std::string tracking_log;
    const auto tracking_result = motion_service_->start_tracking(tracking_log);
    status.state = tracking_result.ok ? WorkflowState::Completed : WorkflowState::Error;
    status.progress = tracking_result.ok ? 1.0 : 0.0;
    status.message = tracking_result.ok ? "target workflow completed" : tracking_log;
    status.stage = workflowStateToLegacyStage(status.state);
    status.steps.push_back({"tracking", tracking_result.ok, tracking_log});

    status.ok = tracking_result.ok;
    return status;
}

ObservationService::TargetWorkflowStatus ObservationService::abortTargetWorkflow(const catalog::ObservationContext& context) {
    // Workflow orchestration: stop the active workflow and clear any session-scoped state.
    // This path also assumes no concurrent workflow is mutating the same session state.
    // TODO: If multiple workflows can run simultaneously, protect the abort path so it cannot
    // race with runTargetWorkflow or with current-target updates.
    (void)context;
    TargetWorkflowStatus status;
    status.state = WorkflowState::Aborted;
    status.progress = 0.0;
    status.stage = workflowStateToLegacyStage(status.state);
    status.message = "target workflow aborted";
    status.steps.push_back({"abort", true, status.message});
    if (motion_service_) {
        std::string log;
        const auto result = motion_service_->abort(log);
        status.monitoring_message = log;
        status.steps.push_back({"abort", result.ok, log});
    }
    clearCurrentTarget();
    return status;
}

} // namespace asdevlab
