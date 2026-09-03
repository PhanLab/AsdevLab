#include "asdevlab/services/target_service.hpp"
#include "asdevlab/astronomy/astronomy_service.hpp"
#include "asdevlab/astronomy/providers/libnova_ephemeris_provider.hpp"
#include "asdevlab/catalog/observation_context.hpp"
#include "asdevlab/safety_service.hpp"
#include "asdevlab/state_machine.hpp"

#include <cassert>
#include <cmath>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>

namespace asdevlab {
namespace {

class CapturingMount : public hardware::MountInterface {
public:
    double last_ra = 0.0;
    double last_dec = 0.0;
    bool goto_called = false;
    hardware::MountGotoRequest last_goto_request{};
    bool start_called = false;
    bool should_succeed = true;
    bool goto_response_ok = true;
    bool start_tracking_ok = true;
    bool abort_ok = true;
    hardware::MountMode detected_mode = hardware::MountMode::Equatorial;

    bool setFlipMirrorCamera() override { return true; }
    bool setFlipMirrorEyepiece() override { return true; }
    hardware::FlipMirrorPosition getFlipMirrorState() override { return hardware::FlipMirrorPosition::Unknown; }

    bool connect() override { return true; }
    bool gotoRaDec(double ra, double dec) override {
        goto_called = true;
        last_ra = ra;
        last_dec = dec;
        return should_succeed;
    }
    bool syncRaDec(double, double) override { return should_succeed; }
    bool startTracking() override { start_called = true; return start_tracking_ok; }
    bool stopTracking() override { return true; }
    bool park() override { return true; }
    bool home() override { return true; }
    bool abort() override { return abort_ok; }
    std::string getStatus() override { return "ok"; }

    std::optional<hardware::MountStatus> readMountStatus() override {
        return hardware::MountStatus{"ok", true};
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
        response.status = hardware::MountStatus{"ok", true};
        return response;
    }

    hardware::MountGotoResponse gotoTarget(const hardware::MountGotoRequest& request) override {
        goto_called = true;
        last_goto_request = request;
        last_ra = request.ra_hours;
        last_dec = request.dec_deg;
        hardware::MountGotoResponse response;
        response.ok = goto_response_ok;
        response.error = goto_response_ok ? hardware::MountError::None : hardware::MountError::Communication;
        return response;
    }

    hardware::MountModeResponse getMountMode(const hardware::MountModeRequest&) override {
        hardware::MountModeResponse response;
        response.ok = true;
        response.supported = true;
        response.mode = detected_mode;
        response.error = hardware::MountError::None;
        return response;
    }

    hardware::MountSyncResponse syncTarget(const hardware::MountSyncRequest&) override {
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

struct WorkflowHarness {
    std::string catalog_path;
    catalog::CatalogEngine catalog;
    SafetyService safety;
    StateMachine state_machine;
    CapturingMount mount;
    ObservationService observation_service;
    std::shared_ptr<astronomy::LibnovaEphemerisProvider> astronomy_provider;
    astronomy::AstronomyService astronomy_service;
    MotionService motion;
    catalog::CoordinateResolver resolver;
    TargetService target;

    explicit WorkflowHarness(const std::string& path)
        : catalog_path(path)
        , astronomy_provider(std::make_shared<astronomy::LibnovaEphemerisProvider>())
        , astronomy_service(astronomy_provider)
        , motion(safety, state_machine, mount, observation_service, astronomy_service)
        , resolver(nullptr)
        , target(catalog, resolver, motion) {
        assert(catalog.loadCatalog(catalog_path));
        observation_service.setTargetService(target);
        observation_service.setMotionService(motion);
        observation_service.setAstronomyService(astronomy_service);
    }

    catalog::ObservationContext makeContext(double latitude_deg = 0.0, double longitude_deg = 0.0, double elevation_m = 0.0, std::time_t utc = 1735689600) const {
        catalog::ObservationContext context;
        context.utc_time = std::chrono::system_clock::from_time_t(utc);
        context.latitude_degrees = latitude_deg;
        context.longitude_degrees = longitude_deg;
        context.elevation_meters = elevation_m;
        context.pressure_mbar = 0.0;
        context.temperature_c = 0.0;
        context.timezone = "UTC";
        return context;
    }
};

} // namespace
} // namespace asdevlab

int main() {
    const std::string path = "catalog_target_test.json";
    std::ofstream out(path);
    out << R"({
  "name": "core",
  "objects": [
    {
      "id": "M31",
      "name": "Andromeda Galaxy",
      "provider": "static",
      "type": "Galaxy",
      "magnitude": 3.44,
      "ra": 0.712,
      "dec": 41.269
    },
    {
      "id": "BAD",
      "name": "Bad Target",
      "provider": "tle",
      "type": "Satellite"
    }
  ]
})";
    out.close();

    asdevlab::WorkflowHarness harness(path);

    auto sr = harness.target.search("M31");
    assert(sr.results.size() == 1);
    assert(sr.results.front().id == "M31");

    auto ctx = harness.makeContext();
    auto res = harness.target.gotoObject("M31", ctx);
    assert(res.ok);
    assert(harness.mount.goto_called);
    assert(fabs(harness.mount.last_ra - 0.712) < 1e-6);
    assert(fabs(harness.mount.last_dec - 41.269) < 1e-6);
    assert(fabs(harness.mount.last_goto_request.ra_hours - 0.712) < 1e-6);
    assert(fabs(harness.mount.last_goto_request.dec_deg - 41.269) < 1e-6);
    assert(!harness.mount.last_goto_request.use_alt_az);

    harness.mount.goto_called = false;
    harness.mount.last_goto_request = {};
    harness.mount.detected_mode = asdevlab::hardware::MountMode::AltAz;
    auto altaz_ctx = harness.makeContext(20.45, 106.33, 100.0, 1735689600);
    auto altaz_res = harness.target.gotoObject("M31", altaz_ctx);
    assert(altaz_res.ok);
    assert(harness.mount.goto_called);
    assert(harness.mount.last_goto_request.use_alt_az);
    assert(harness.mount.last_goto_request.alt_deg > -90.0);
    assert(harness.mount.last_goto_request.alt_deg < 90.0);
    assert(harness.mount.last_goto_request.az_deg >= 0.0);
    assert(harness.mount.last_goto_request.az_deg < 360.0);
    assert(fabs(harness.mount.last_goto_request.alt_deg) > 1e-6 || fabs(harness.mount.last_goto_request.az_deg) > 1e-6);

    harness.mount.goto_called = false;
    auto unresolved = harness.target.gotoObject("BAD", altaz_ctx);
    assert(!unresolved.ok);
    assert(!harness.mount.goto_called);

    const auto workflow = harness.observation_service.runTargetWorkflow("M31", altaz_ctx, true);
    assert(workflow.state == asdevlab::ObservationService::WorkflowState::Completed);
    assert(workflow.progress > 0.0);
    assert(workflow.message.find("completed") != std::string::npos || workflow.message.find("tracking") != std::string::npos);

    auto search_result = harness.target.search("M31");
    assert(search_result.results.size() == 1);

    auto resolve_result = harness.target.resolveTarget("M31", altaz_ctx);
    assert(resolve_result.ok);
    assert(resolve_result.target.resolved);

    auto preview_result = harness.target.previewObject("M31", altaz_ctx);
    assert(preview_result.ok);

    auto invalid_preview = harness.target.previewObject("BAD", altaz_ctx);
    assert(!invalid_preview.ok);
    assert(invalid_preview.log == "object could not be resolved");

    const auto aborted = harness.observation_service.abortTargetWorkflow(altaz_ctx);
    assert(aborted.state == asdevlab::ObservationService::WorkflowState::Aborted);

    auto invalid_target = harness.observation_service.runTargetWorkflow("NOPE", altaz_ctx, true);
    assert(invalid_target.state == asdevlab::ObservationService::WorkflowState::Error);
    assert(invalid_target.progress == 0.0);

    auto below_horizon = harness.observation_service.runTargetWorkflow("M31", harness.makeContext(-90.0, 0.0, 0.0, 1735689600), true);
    assert(below_horizon.state == asdevlab::ObservationService::WorkflowState::Error);
    assert(below_horizon.message.find("visible") != std::string::npos);

    harness.state_machine.requestTransition(asdevlab::SystemState::SLEWING, asdevlab::Owner::GotoService);
    auto mount_busy = harness.observation_service.runTargetWorkflow("M31", altaz_ctx, true);
    assert(mount_busy.state == asdevlab::ObservationService::WorkflowState::Error);

    harness.mount.goto_response_ok = false;
    auto disconnected_mount = harness.observation_service.runTargetWorkflow("M31", altaz_ctx, true);
    assert(disconnected_mount.state == asdevlab::ObservationService::WorkflowState::Error);

    std::remove(path.c_str());
    std::cout << "target service test passed\n";
    return 0;
}
