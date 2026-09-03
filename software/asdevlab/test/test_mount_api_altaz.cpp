#include "asdevlab/web/mount_api.hpp"
#include "asdevlab/services/motion_service.hpp"
#include "asdevlab/safety_service.hpp"
#include "asdevlab/state_machine.hpp"
#include "asdevlab/services/observation_service.hpp"
#include "asdevlab/astronomy/providers/libnova_ephemeris_provider.hpp"

#include <cassert>
#include <iostream>

using namespace asdevlab;

class FakeMountForApi : public hardware::MountInterface {
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

    hardware::MountGotoRequest last_goto{};

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

    hardware::MountGotoResponse gotoTarget(const hardware::MountGotoRequest& request) override {
        goto_target_called = true;
        last_goto = request;
        hardware::MountGotoResponse response;
        response.ok = true;
        return response;
    }

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

int main() {
    // Parsing tests
    {
        auto p = asdevlab::web::parse_goto_body("{\"ra\":\"1.5\",\"dec\":\"10.0\"}");
        assert(p.type == asdevlab::web::ParsedGoto::Type::RADEC);
        assert(p.ra_hours == 1.5);
        assert(p.dec_deg == 10.0);
    }
    {
        auto p = asdevlab::web::parse_goto_body("{\"alt\":45.0,\"az\":90.0}");
        assert(p.type == asdevlab::web::ParsedGoto::Type::ALTAZ);
        assert(p.alt_deg == 45.0);
        assert(p.az_deg == 90.0);
    }
    {
        auto p = asdevlab::web::parse_goto_body("{\"alt\":45.0}");
        assert(p.type == asdevlab::web::ParsedGoto::Type::Invalid);
    }
    {
        auto p = asdevlab::web::parse_goto_body("{\"ra\":\"1.5\"}");
        assert(p.type == asdevlab::web::ParsedGoto::Type::Invalid);
    }
    {
        auto p = asdevlab::web::parse_goto_body("{\"alt\":\"nonnumeric\",\"az\":90}");
        assert(p.type == asdevlab::web::ParsedGoto::Type::Invalid);
    }
    {
        auto p = asdevlab::web::parse_goto_body("{}");
        assert(p.type == asdevlab::web::ParsedGoto::Type::Invalid);
    }

    // MotionService -> MountInterface routing for alt/az
    asdevlab::SafetyService safety;
    asdevlab::StateMachine state_machine;
    FakeMountForApi mount;
    asdevlab::ObservationService observation_service;
    auto astronomy_provider = std::make_shared<asdevlab::astronomy::LibnovaEphemerisProvider>();
    asdevlab::astronomy::AstronomyService astronomy_service(astronomy_provider);
    asdevlab::MotionService motion_service(safety, state_machine, mount, observation_service, astronomy_service);

    std::string log;
    // Call alt/az goto via MotionService
    const auto r = motion_service.goto_target(0.0, 0.0, true, 45.0, 90.0, log);
    assert(r.ok);
    assert(mount.goto_target_called);
    assert(mount.last_goto.use_alt_az);
    assert(mount.last_goto.alt_deg == 45.0);
    assert(mount.last_goto.az_deg == 90.0);

    // Call alt/az sync via MotionService
    const auto r2 = motion_service.sync(0.0, 0.0, true, 10.0, 20.0, log);
    assert(r2.ok);
    assert(mount.sync_target_called);

    std::cout << "mount api altaz tests passed\n";
    return 0;
}
