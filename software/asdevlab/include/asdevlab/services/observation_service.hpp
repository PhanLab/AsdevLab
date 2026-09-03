#pragma once

#include "asdevlab/catalog/catalog_object.hpp"
#include "asdevlab/catalog/observation_context.hpp"
#include "asdevlab/catalog/resolved_target.hpp"

#include <optional>
#include <string>
#include <vector>

namespace asdevlab {

class MotionService;
class TargetService;
namespace astronomy {
class AstronomyService;
}

class ObservationService {
public:
    // Current implementation assumes a single workflow runs at a time.
    // No mutexes are used today; callers are expected to serialize workflow entry points.
    // TODO: If concurrent workflows are introduced, protect workflow state updates and
    // current-target access across runTargetWorkflow/abortTargetWorkflow calls.
    enum class WorkflowState {
        Idle,
        Searching,
        Resolving,
        Preview,
        VisibilityCheck,
        Slewing,
        Tracking,
        Completed,
        Aborted,
        Error
    };

    struct WorkflowStep {
        std::string name;
        bool ok = false;
        std::string message;
    };

    struct TargetWorkflowStatus {
        WorkflowState state = WorkflowState::Idle;
        bool ok = false;
        std::string message;
        double progress = 0.0;
        std::optional<catalog::CatalogObject> catalog_object;
        catalog::ResolvedTarget resolved_target;
        std::vector<WorkflowStep> steps;
        std::string stage;
        std::string monitoring_message;

        std::string stateName() const {
            switch (state) {
                case WorkflowState::Idle: return "Idle";
                case WorkflowState::Searching: return "Searching";
                case WorkflowState::Resolving: return "Resolving";
                case WorkflowState::Preview: return "Preview";
                case WorkflowState::VisibilityCheck: return "VisibilityCheck";
                case WorkflowState::Slewing: return "Slewing";
                case WorkflowState::Tracking: return "Tracking";
                case WorkflowState::Completed: return "Completed";
                case WorkflowState::Aborted: return "Aborted";
                case WorkflowState::Error: return "Error";
            }
            return "Idle";
        }

        std::string legacyStage() const {
            switch (state) {
                case WorkflowState::Searching: return "search";
                case WorkflowState::Resolving: return "resolve";
                case WorkflowState::Preview: return "preview";
                case WorkflowState::VisibilityCheck: return "visibility";
                case WorkflowState::Slewing: return "goto";
                case WorkflowState::Tracking: return "tracking";
                case WorkflowState::Completed: return "tracking";
                case WorkflowState::Aborted: return "abort";
                case WorkflowState::Error: return "error";
                case WorkflowState::Idle:
                default:
                    return "idle";
            }
        }

        bool isPreviewed() const {
            return state == WorkflowState::Preview || state == WorkflowState::VisibilityCheck || state == WorkflowState::Slewing || state == WorkflowState::Tracking || state == WorkflowState::Completed;
        }

        bool isGotoDispatched() const {
            return state == WorkflowState::Slewing || state == WorkflowState::Tracking || state == WorkflowState::Completed;
        }

        bool isTrackingActive() const {
            return state == WorkflowState::Tracking || state == WorkflowState::Completed;
        }

        bool isAborted() const {
            return state == WorkflowState::Aborted;
        }
    };

    ObservationService() = default;

    // Workflow orchestration entry points.
    void setTargetService(TargetService& service);
    void setMotionService(MotionService& service);
    void setAstronomyService(astronomy::AstronomyService& service);

    catalog::ObservationContext makeObservationContext() const;
    TargetWorkflowStatus runTargetWorkflow(const std::string& target_query,
                                           const catalog::ObservationContext& context,
                                           bool require_visibility = true);
    TargetWorkflowStatus abortTargetWorkflow(const catalog::ObservationContext& context);

    // Observation-session state kept here for now.
    // TODO: If a dedicated session abstraction is introduced later, this current-target state
    // is a good candidate to move out of the orchestration service while preserving the API.
    void setCurrentTarget(const std::string& target_id);
    void clearCurrentTarget();
    std::string currentTargetId() const;

private:
    // Workflow-orchestration dependencies.
    TargetService* target_service_ = nullptr;
    MotionService* motion_service_ = nullptr;
    astronomy::AstronomyService* astronomy_service_ = nullptr;

    // TODO: This is session-scoped state and should be considered for future extraction.
    // If concurrent workflows are allowed later, this member will need protection alongside
    // the workflow status fields that are mutated during orchestration.
    std::string current_target_id_;
};

} // namespace asdevlab
