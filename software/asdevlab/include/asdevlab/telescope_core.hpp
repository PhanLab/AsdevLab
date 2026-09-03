#pragma once

#include "asdevlab/services/motion_service.hpp"
#include "asdevlab/services/tracking_service.hpp"
#include "asdevlab/services/guide_service.hpp"
#include "asdevlab/services/alignment_service.hpp"
#include "asdevlab/services/camera_service.hpp"
#include "asdevlab/services/image_service.hpp"
#include "asdevlab/services/plate_solver_service.hpp"
#include "asdevlab/services/scheduler_service.hpp"
#include "asdevlab/services/focus_service.hpp"
#include "asdevlab/astronomy/astronomy_service.hpp"
#include "asdevlab/services/target_service.hpp"
#include "asdevlab/services/observation_service.hpp"
#include "asdevlab/hardware/mount/onstep_mount_client.hpp"
#include "asdevlab/safety_service.hpp"
#include "asdevlab/state_machine.hpp"

namespace asdevlab {

class TelescopeCore {
public:
    TelescopeCore();

    hardware::MountModeResponse selectMountMode(const hardware::MountModeRequest& request);
    hardware::MountMode getDesiredMountMode() const;
    void setDesiredMountMode(hardware::MountMode mode);
    bool isMountModeMatched() const;
    bool hasPendingMountModeSelection() const;

    MotionService& motion();
    TargetService& target();
    ObservationService& observation();
    catalog::CatalogEngine& catalog();
    TrackingService& tracking();
    GuideService& guide();
    AlignmentService& alignment();
    CameraService& camera();
    ImageService& image();
    PlateSolverService& plate_solver();
    SchedulerService& scheduler();
    FocusService& focus();
    SafetyService& safety();
    StateMachine& state_machine();

    // Low-level mount access wrappers (convenience pass-throughs to the
    // underlying mount client used by this core). These avoid exposing the
    // concrete client type while allowing web API handlers to obtain typed
    // mount responses without duplicating mount logic.
    hardware::MountStatusResponse mountStatus(const hardware::MountStatusRequest& request = hardware::MountStatusRequest());
    hardware::MountCoordinatesResponse mountCoordinates(const hardware::MountCoordinatesRequest& request = hardware::MountCoordinatesRequest());
    hardware::MountModeResponse mountMode(const hardware::MountModeRequest& request = hardware::MountModeRequest());
    hardware::MountTrackingResponse mountTracking(const hardware::MountTrackingRequest& request = hardware::MountTrackingRequest());
    hardware::MountFocuserResponse mountFocuser(const hardware::MountFocuserRequest& request = hardware::MountFocuserRequest());
    hardware::MountRotatorResponse mountRotator(const hardware::MountRotatorRequest& request = hardware::MountRotatorRequest());
    hardware::MountFlipMirrorResponse mountFlipMirror(const hardware::MountFlipMirrorRequest& request = hardware::MountFlipMirrorRequest());

private:
    SafetyService safety_service_;
    StateMachine state_machine_;
    // ASDEVLAB is currently scoped to a single OnStepX LX200 mount backend.
    hardware::OnStepMountClient mount_client_;
    hardware::MountMode desired_mount_mode_ = hardware::MountMode::AltAz;
    bool pending_mount_mode_selection_ = false;
    MotionService motion_service_;
    TrackingService tracking_service_;
    GuideService guide_service_;
    AlignmentService alignment_service_;
    CameraService camera_service_;
    ImageService image_service_;
    PlateSolverService plate_solver_service_;
    SchedulerService scheduler_service_;
    FocusService focus_service_;
    catalog::CatalogEngine catalog_engine_;
    catalog::CoordinateResolver coordinate_resolver_{nullptr};
    astronomy::AstronomyService astronomy_service_;
    TargetService target_service_;
    ObservationService observation_service_;
};

} // namespace asdevlab
