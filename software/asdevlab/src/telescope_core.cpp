#include "asdevlab/telescope_core.hpp"
#include "asdevlab/hardware/mount/mount_config.hpp"

#include <string>

namespace asdevlab {

namespace {

hardware::MountModeResponse makeMountModeResponseForSelection(hardware::MountMode mode, bool ok, const std::string& message) {
    hardware::MountModeResponse response;
    response.ok = ok;
    response.mode = mode;
    response.message = message;
    response.error = ok ? hardware::MountError::None : hardware::MountError::InvalidRequest;
    return response;
}

} // namespace

TelescopeCore::TelescopeCore()
    : safety_service_()
    , state_machine_()
    , mount_client_(hardware::MountConfig::fromEnvironment())
    , observation_service_()
    , astronomy_service_(std::make_shared<astronomy::LibnovaEphemerisProvider>())
    , motion_service_(safety_service_, state_machine_, mount_client_, observation_service_, astronomy_service_)
    , catalog_engine_()
    , coordinate_resolver_(nullptr)
    , target_service_(catalog_engine_, coordinate_resolver_, motion_service_)
    , tracking_service_()
    , guide_service_(safety_service_, state_machine_)
    , alignment_service_()
    , camera_service_()
    , image_service_()
    , plate_solver_service_()
    , scheduler_service_(safety_service_, state_machine_, motion_service_, camera_service_, plate_solver_service_)
    , focus_service_(safety_service_, state_machine_)
{
    observation_service_.setTargetService(target_service_);
    observation_service_.setMotionService(motion_service_);
    observation_service_.setAstronomyService(astronomy_service_);
}

hardware::MountModeResponse TelescopeCore::selectMountMode(const hardware::MountModeRequest& request) {
    if (!request.mode.has_value()) {
        return makeMountModeResponseForSelection(hardware::MountMode::AltAz, false, "no mount mode specified");
    }

    desired_mount_mode_ = request.mode.value();
    pending_mount_mode_selection_ = !request.apply;

    if (!request.apply) {
        return makeMountModeResponseForSelection(
            desired_mount_mode_,
            true,
            "mount mode selected; firmware rebuild and upload required");
    }

    if (!mount_client_.connect()) {
        hardware::MountModeResponse response;
        response.error = hardware::MountError::Communication;
        response.mode = desired_mount_mode_;
        response.message = "failed to reconnect after firmware upload";
        pending_mount_mode_selection_ = false;
        return response;
    }

    mount_client_.updateDetectedMountMode();
    const bool matched = mount_client_.getDetectedMountMode() == desired_mount_mode_;
    hardware::MountModeResponse response;
    response.ok = matched;
    response.mode = mount_client_.getDetectedMountMode();
    response.message = matched ? "mount mode verified after firmware upload" : "firmware mismatch after upload";
    response.error = matched ? hardware::MountError::None : hardware::MountError::InvalidResponse;
    pending_mount_mode_selection_ = false;
    return response;
}

hardware::MountMode TelescopeCore::getDesiredMountMode() const {
    return desired_mount_mode_;
}

void TelescopeCore::setDesiredMountMode(hardware::MountMode mode) {
    desired_mount_mode_ = mode;
    pending_mount_mode_selection_ = true;
}

bool TelescopeCore::isMountModeMatched() const {
    return desired_mount_mode_ == mount_client_.getDetectedMountMode();
}

bool TelescopeCore::hasPendingMountModeSelection() const {
    return pending_mount_mode_selection_;
}

MotionService& TelescopeCore::motion() { return motion_service_; }
TrackingService& TelescopeCore::tracking() { return tracking_service_; }
GuideService& TelescopeCore::guide() { return guide_service_; }
AlignmentService& TelescopeCore::alignment() { return alignment_service_; }
CameraService& TelescopeCore::camera() { return camera_service_; }
ImageService& TelescopeCore::image() { return image_service_; }
PlateSolverService& TelescopeCore::plate_solver() { return plate_solver_service_; }
SchedulerService& TelescopeCore::scheduler() { return scheduler_service_; }
FocusService& TelescopeCore::focus() { return focus_service_; }
TargetService& TelescopeCore::target() { return target_service_; }
ObservationService& TelescopeCore::observation() { return observation_service_; }
catalog::CatalogEngine& TelescopeCore::catalog() { return catalog_engine_; }
SafetyService& TelescopeCore::safety() { return safety_service_; }
StateMachine& TelescopeCore::state_machine() { return state_machine_; }

hardware::MountStatusResponse TelescopeCore::mountStatus(const hardware::MountStatusRequest& request) {
    return mount_client_.getStatus(request);
}

hardware::MountCoordinatesResponse TelescopeCore::mountCoordinates(const hardware::MountCoordinatesRequest& request) {
    return mount_client_.getCoordinates(request);
}

hardware::MountModeResponse TelescopeCore::mountMode(const hardware::MountModeRequest& request) {
    return mount_client_.getMountMode(request);
}

hardware::MountTrackingResponse TelescopeCore::mountTracking(const hardware::MountTrackingRequest& request) {
    return mount_client_.getTracking(request);
}

hardware::MountFocuserResponse TelescopeCore::mountFocuser(const hardware::MountFocuserRequest& request) {
    return mount_client_.getFocuser(request);
}

hardware::MountRotatorResponse TelescopeCore::mountRotator(const hardware::MountRotatorRequest& request) {
    return mount_client_.getRotator(request);
}

hardware::MountFlipMirrorResponse TelescopeCore::mountFlipMirror(const hardware::MountFlipMirrorRequest& request) {
    return mount_client_.moveFlipMirror(request);
}

} // namespace asdevlab
