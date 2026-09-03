#include "asdevlab/astronomy/providers/libnova_ephemeris_provider.hpp"
#include "asdevlab/hardware/mount/mount_interface.hpp"
#include "asdevlab/safety_service.hpp"
#include "asdevlab/services/motion_service.hpp"
#include "asdevlab/state_machine.hpp"

#include <cassert>
#include <iostream>
#include <string>

namespace asdevlab {
namespace {

class FakeMount : public hardware::MountInterface {
public:
    bool connect_called = false;
    bool goto_called = false;
    bool sync_called = false;
    bool goto_target_called = false;
    bool sync_target_called = false;
    bool start_called = false;
    bool stop_called = false;
    bool park_called = false;
    bool home_called = false;
    bool abort_called = false;
    bool should_succeed = true;

    bool setFlipMirrorCamera() override {
        return true;
    }

    bool setFlipMirrorEyepiece() override {
        return true;
    }

    hardware::FlipMirrorPosition getFlipMirrorState() override {
        return hardware::FlipMirrorPosition::Unknown;
    }

    bool connect() override {
        connect_called = true;
        return true;
    }

    bool gotoRaDec(double, double) override {
        goto_called = true;
        return should_succeed;
    }

    bool syncRaDec(double, double) override {
        sync_called = true;
        return should_succeed;
    }

    bool startTracking() override {
        start_called = true;
        return should_succeed;
    }

    bool stopTracking() override {
        stop_called = true;
        return should_succeed;
    }

    bool park() override {
        park_called = true;
        return should_succeed;
    }

    bool home() override {
        home_called = true;
        return should_succeed;
    }

    bool abort() override {
        abort_called = true;
        return should_succeed;
    }

    std::string getStatus() override {
        return "status:ok";
    }

    std::optional<hardware::MountStatus> readMountStatus() override {
        return hardware::MountStatus{"status:ok", true};
    }

    std::optional<hardware::RightAscension> readRightAscension() override {
        return hardware::RightAscension{1, 30, 0.0, true};
    }

    std::optional<hardware::Declination> readDeclination() override {
        return hardware::Declination{10, 0, 0.0, false, true};
    }

    std::optional<hardware::Coordinates> readCoordinates() override {
        return hardware::Coordinates{hardware::RightAscension{1, 30, 0.0, true}, hardware::Declination{10, 0, 0.0, false, true}, true};
    }

    std::optional<hardware::GotoResult> readGotoState() override {
        return hardware::GotoResult{true, hardware::GotoState::InProgress, "goto in progress"};
    }

    std::vector<hardware::MountCapability> capabilities() const override {
        return {{hardware::MountCapabilityId::Status, true, "status"}};
    }

    bool isCapabilitySupported(hardware::MountCapabilityId) const override { return true; }

    hardware::MountStatusResponse getStatus(const hardware::MountStatusRequest&) override {
        hardware::MountStatusResponse response;
        response.ok = true;
        response.status = hardware::MountStatus{"status:ok", true};
        return response;
    }

    hardware::MountGotoResponse gotoTarget(const hardware::MountGotoRequest&) override {
        goto_target_called = true;
        hardware::MountGotoResponse response;
        response.ok = true;
        return response;
    }

    // buildGotoCommands removed from MountInterface; FakeMount does not
    // implement protocol-specific command formation.

    hardware::MountSyncResponse syncTarget(const hardware::MountSyncRequest&) override {
        sync_target_called = true;
        hardware::MountSyncResponse response;
        response.ok = true;
        return response;
    }

    hardware::MountCoordinatesResponse getCoordinates(const hardware::MountCoordinatesRequest&) override {
        hardware::MountCoordinatesResponse response;
        response.ok = true;
        response.coordinates = readCoordinates().value_or(hardware::Coordinates{});
        return response;
    }

    hardware::MountSlewResponse slew(const hardware::MountSlewRequest&) override {
        hardware::MountSlewResponse response;
        response.ok = true;
        return response;
    }

    hardware::MountTrackingResponse getTracking(const hardware::MountTrackingRequest&) override {
        hardware::MountTrackingResponse response;
        response.ok = true;
        return response;
    }

    hardware::MountGuideResponse guide(const hardware::MountGuideRequest&) override {
        hardware::MountGuideResponse response;
        response.ok = true;
        return response;
    }

    hardware::MountParkResponse getParkStatus(const hardware::MountParkRequest&) override {
        hardware::MountParkResponse response;
        response.ok = true;
        return response;
    }

    hardware::MountHomeResponse getHomeStatus(const hardware::MountHomeRequest&) override {
        hardware::MountHomeResponse response;
        response.ok = true;
        return response;
    }

    hardware::MountSiteResponse getSite(const hardware::MountSiteRequest&) override {
        hardware::MountSiteResponse response;
        response.ok = true;
        return response;
    }

    hardware::MountTimeResponse getTime(const hardware::MountTimeRequest&) override {
        hardware::MountTimeResponse response;
        response.ok = true;
        return response;
    }

    hardware::MountPecResponse getPec(const hardware::MountPecRequest&) override {
        hardware::MountPecResponse response;
        response.ok = true;
        return response;
    }

    hardware::MountLimitsResponse getLimits(const hardware::MountLimitsRequest&) override {
        hardware::MountLimitsResponse response;
        response.ok = true;
        return response;
    }

    hardware::MountFocuserResponse getFocuser(const hardware::MountFocuserRequest&) override {
        hardware::MountFocuserResponse response;
        response.ok = true;
        return response;
    }

    hardware::MountFlipMirrorResponse getFlipMirror(const hardware::MountFlipMirrorRequest&) override {
        hardware::MountFlipMirrorResponse response;
        response.ok = true;
        return response;
    }
};

} // namespace
} // namespace asdevlab

int main() {
    asdevlab::SafetyService safety;
    asdevlab::StateMachine state_machine;
    asdevlab::FakeMount mount;
    asdevlab::ObservationService observation_service;
    auto astronomy_provider = std::make_shared<asdevlab::astronomy::LibnovaEphemerisProvider>();
    asdevlab::astronomy::AstronomyService astronomy_service(astronomy_provider);
    asdevlab::MotionService motion_service(safety, state_machine, mount, observation_service, astronomy_service);

    std::string log;
    const auto goto_result = motion_service.goto_target(1.5, 10.0, log);
    assert(goto_result.ok);
    assert(mount.goto_target_called);
    assert(state_machine.current_state() == asdevlab::SystemState::TRACKING);

    const auto legacy_result = motion_service.goto_target("M31");
    assert(legacy_result.find("TargetService") != std::string::npos);
    assert(legacy_result.find("accepted") != std::string::npos);

    const auto track_result = motion_service.start_tracking(log);
    assert(track_result.ok);
    assert(state_machine.current_state() == asdevlab::SystemState::TRACKING);

    const auto status_result = motion_service.status(log);
    assert(status_result.ok);
    assert(log.find("status:ok") != std::string::npos);

    std::cout << "motion service test passed\n";
    return 0;
}
