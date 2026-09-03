#include "asdevlab/hardware/mount/onstep_mount_client.hpp"
#include "asdevlab/hardware/mount/transport_interface.hpp"
#include "asdevlab/telescope_core.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace asdevlab {
namespace hardware {

class MockTransport : public TransportInterface {
public:
    std::vector<std::string> commands;
    std::vector<std::string> get_requests;
    std::unordered_set<std::string> invalid_responses;
    std::string flip_mirror_state = "1";

    bool send(const std::string& command, std::string& response_out) override {
        commands.push_back(command);
        if (invalid_responses.count(command) != 0U) {
            response_out = "";
            return true;
        }
        if (command == ":GU#" || command == ":Gu#") {
            response_out = "0x03";
        } else if (command == ":GW#") {
            response_out = "1";
        } else if (command == ":GT#") {
            response_out = "0.75";
        } else if (command == ":GR#") {
            response_out = "23:59:59";
        } else if (command == ":GD#") {
            response_out = "+12*34:56";
        } else if (command == ":W?#") {
            response_out = "1";
        } else if (command == ":GL#") {
            response_out = "23:59:59";
        } else if (command == ":GC#") {
            response_out = "2026-07-17";
        } else if (command == ":GS#") {
            response_out = "10:20:30";
        } else if (command == ":SL#") {
            response_out = "+34*56:00";
        } else if (command == ":SG#") {
            response_out = "-122*33:44";
        } else if (command == ":SS#") {
            response_out = "100";
        } else if (command == ":GX80#") {
            response_out = "03:59:59";
        } else if (command == ":GX81#") {
            response_out = "2026-07-17";
        } else if (command == ":GX90#") {
            response_out = "0.5";
        } else if (command == ":GX93#") {
            response_out = "1.5";
        } else if (command == ":TQ#") {
            response_out = "0.75";
        } else if (command == ":h?#") {
            response_out = "1,1,180,240";
        } else if (command == ":hC#") {
            response_out = "1";
        } else if (command == ":hQ#") {
            response_out = "1";
        } else if (command == ":hR#") {
            response_out = "0";
        } else if (command == ":$QZ+#") {
            response_out = "";
        } else if (command == ":$QZ-#") {
            response_out = "";
        } else if (command == ":$QZ?#") {
            response_out = "1";
        } else if (command == ":VS#") {
            response_out = "1";
        } else if (command == ":Gh#") {
            response_out = "10";
        } else if (command == ":Go#") {
            response_out = "20";
        } else if (command == ":GXE9#") {
            response_out = "30";
        } else if (command == ":FG#") {
            response_out = "125";
        } else if (command == ":FT#") {
            response_out = "S2";
        } else if (command == ":F?#") {
            response_out = "1";
        } else if (command == ":FM0#") {
            flip_mirror_state = "0";
            response_out = "OK";
        } else if (command == ":FM1#") {
            flip_mirror_state = "1";
            response_out = "OK";
        } else if (command == ":FM?#") {
            response_out = flip_mirror_state;
        } else if (command == ":GA#") {
            response_out = "+45*00:00";
        } else if (command == ":GZ#") {
            response_out = "180*00:00";
        } else if (command == ":GX9A#") {
            response_out = "12.3";
        } else if (command == ":GX9B#") {
            response_out = "1013.2";
        } else if (command == ":GX9C#") {
            response_out = "45.6";
        } else if (command == ":GX9E#") {
            response_out = "5.0";
        } else if (command == ":GX9F#") {
            response_out = "60.0";
        } else if (command == ":GX98#") {
            response_out = "R";
        } else if (command == ":rG#") {
            response_out = "+10*00:00";
        } else if (command == ":r>#") {
            response_out = "1";
        } else if (command == ":r<#") {
            response_out = "1";
        } else if (command == ":rQ#") {
            response_out = "1";
        } else if (command == ":rZ#") {
            response_out = "1";
        } else if (command == ":rF#") {
            response_out = "1";
        } else if (command == ":rC#") {
            response_out = "1";
        } else if (command.size() >= 3 && command.rfind(":r", 0) == 0 && (command[2] >= '1' && command[2] <= '9') && command.back() == '#') {
            // rate set like :r1#..:r9#
            response_out = "OK";
        } else if (command.rfind(":rS", 0) == 0 || command.rfind(":rr", 0) == 0) {
            response_out = "OK";
        } else if (command == ":GXU3#") {
            response_out = "ST,OA,OB,GA,GB,OT,PW,GF";
        } else if (command == ":GXM#") {
            response_out = "0#";
        } else {
            response_out = "OK";
        }
        return true;
    }

    bool sendGet(const std::string& resource,
                 const std::vector<std::pair<std::string, std::string>>& query_params,
                 std::string& response_out) override {
        std::ostringstream request;
        request << resource;
        if (!query_params.empty()) {
            request << "?";
            bool first = true;
            for (const auto& param : query_params) {
                if (!first) request << "&";
                first = false;
                request << param.first << "=" << param.second;
            }
        }
        get_requests.push_back(request.str());
        response_out = "OK";
        return true;
    }
};

} // namespace hardware
} // namespace asdevlab

int main() {
    using namespace asdevlab::hardware;
    setenv("ASDEVLAB_LATITUDE", "34.5", 1);
    setenv("ASDEVLAB_LONGITUDE", "-118.25", 1);
    setenv("ASDEVLAB_ELEVATION", "89", 1);
    setenv("ASDEVLAB_LOCAL_DATE", "2026-07-17", 1);
    setenv("ASDEVLAB_LOCAL_TIME", "23:59:59", 1);
    setenv("ASDEVLAB_UTC_OFFSET", "-8.5", 1);

    auto mock = std::make_unique<MockTransport>();
    MockTransport* mock_ptr = mock.get();
    OnStepMountClient client(std::move(mock));

    assert(client.connect());
    assert(mock_ptr->commands.size() >= 6);
    assert(std::find(mock_ptr->commands.begin(), mock_ptr->commands.end(), ":St+34*30:00#") != mock_ptr->commands.end());
    assert(std::find(mock_ptr->commands.begin(), mock_ptr->commands.end(), ":Sg-118*15:00#") != mock_ptr->commands.end());
    assert(std::find(mock_ptr->commands.begin(), mock_ptr->commands.end(), ":Sv89#") != mock_ptr->commands.end());
    assert(std::find(mock_ptr->commands.begin(), mock_ptr->commands.end(), ":SC2026-07-17#") != mock_ptr->commands.end());
    assert(std::find(mock_ptr->commands.begin(), mock_ptr->commands.end(), ":SL23:59:59#") != mock_ptr->commands.end());
    assert(std::find(mock_ptr->commands.begin(), mock_ptr->commands.end(), ":SG-08:30#") != mock_ptr->commands.end());

    bool ok = client.gotoRaDec(1.234, -5.67);
    assert(ok);
    assert(!mock_ptr->commands.empty());
    assert(mock_ptr->commands.back() == ":MS#");
    assert(mock_ptr->get_requests.empty());

    mock_ptr->commands.clear();

    ok = client.syncRaDec(1.234, -5.67);
    assert(ok);
    assert(mock_ptr->commands.back() == ":CM#");

    mock_ptr->commands.clear();

    ok = client.startTracking();
    assert(ok);
    assert(mock_ptr->commands.back() == ":Te#");

    mock_ptr->commands.clear();

    ok = client.stopTracking();
    assert(ok);
    assert(mock_ptr->commands.back() == ":Td#");

    mock_ptr->commands.clear();

    ok = client.park();
    assert(ok);
    assert(mock_ptr->commands.back() == ":hP#");

    mock_ptr->commands.clear();

    ok = client.home();
    assert(ok);
    assert(mock_ptr->commands.back() == ":hF#");

    mock_ptr->commands.clear();

    ok = client.abort();
    assert(ok);
    assert(mock_ptr->commands.back() == ":Q#");

    mock_ptr->commands.clear();

    std::string status = client.getStatus();
    assert(status.find("0x03") != std::string::npos || status.find("0") != std::string::npos);
    assert(mock_ptr->commands.back() == ":Gu#" || mock_ptr->commands.back() == ":GU#");

    const auto capabilities = client.capabilities();
    assert(!capabilities.empty());
    assert(client.isCapabilitySupported(MountCapabilityId::Status));

    const auto typed_status = client.getStatus(MountStatusRequest{});
    assert(typed_status.ok);
    assert(typed_status.supported);
    assert(typed_status.status.tracking_enabled);
    const auto status_module = client.status(MountStatusRequest{});
    assert(status_module.ok);

    const auto coordinates = client.coordinates(MountCoordinatesRequest{});
    assert(coordinates.ok);

    const auto goto_response = client.gotoTarget(MountGotoRequest{1.234, -5.67});
    assert(goto_response.ok);

    mock_ptr->commands.clear();
    const auto altaz_goto_response = client.gotoTarget(MountGotoRequest{0.0, 0.0, true, 12.5, 245.0});
    assert(altaz_goto_response.ok);
    assert(std::find(mock_ptr->commands.begin(), mock_ptr->commands.end(), ":Sa12.50#") != mock_ptr->commands.end());
    assert(std::find(mock_ptr->commands.begin(), mock_ptr->commands.end(), ":Sz245.00#") != mock_ptr->commands.end());
    assert(mock_ptr->commands.back() == ":MS#");

    const auto sync_response = client.syncTarget(MountSyncRequest{1.234, -5.67});
    assert(sync_response.ok);

    const auto safety = client.safety(MountSafetyRequest{true});
    assert(safety.ok);

    const auto tracking = client.getTracking(MountTrackingRequest{});
    assert(tracking.ok);
    assert(tracking.tracking.enabled);

    const auto site = client.getSite(MountSiteRequest{});
    assert(site.ok);
    assert(site.supported);
    assert(site.site.valid);
    assert(site.site.latitude_deg == 34.5);
    assert(site.site.longitude_deg == -118.25);
    assert(site.site.elevation_m == 89.0);
    const auto site_module = client.site(MountSiteRequest{});
    assert(site_module.ok);

    const auto time = client.getTime(MountTimeRequest{});
    assert(time.ok);
    assert(time.time.valid);
    assert(time.time.local_date == "2026-07-17");
    assert(time.time.local_time == "23:59:59");
    const auto time_module = client.time(MountTimeRequest{});
    assert(time_module.ok);

    const auto pec = client.getPec(MountPecRequest{});
    assert(pec.ok);
    assert(pec.pec.valid);
    const auto pec_module = client.pec(MountPecRequest{});
    assert(pec_module.ok);

    const auto pec_enable = client.getPec(MountPecRequest{true, true, false});
    assert(pec_enable.ok);
    assert(mock_ptr->commands.size() >= 2 && mock_ptr->commands[mock_ptr->commands.size() - 2] == ":$QZ+#");

    const auto pec_disable = client.getPec(MountPecRequest{true, false, true});
    assert(pec_disable.ok);
    assert(mock_ptr->commands.size() >= 2 && mock_ptr->commands[mock_ptr->commands.size() - 2] == ":$QZ-#");

    const auto limits = client.getLimits(MountLimitsRequest{});
    assert(limits.ok);
    assert(limits.limits.valid);
    assert(limits.limits.horizon_deg == 10.0);
    assert(limits.limits.overhead_deg == 20.0);
    assert(limits.limits.meridian_deg == 30.0);

    const auto limits_module = client.limits(MountLimitsRequest{});
    assert(limits_module.ok);

    const auto limit_write = client.getLimits(MountLimitsRequest{false, 12.0, 65.0, 90.0});
    assert(limit_write.ok);
    assert(limit_write.limits.valid);
    assert(limit_write.limits.horizon_deg == 12.0);
    assert(limit_write.limits.overhead_deg == 65.0);
    assert(limit_write.limits.meridian_deg == 90.0);
    assert(mock_ptr->commands.end()[-3] == ":Sh12#");
    assert(mock_ptr->commands.end()[-2] == ":So65#");
    assert(mock_ptr->commands.end()[-1] == ":SXE9,90#");

    const auto focuser = client.getFocuser(MountFocuserRequest{});
    assert(focuser.ok);
    assert(focuser.focuser.valid);
    assert(focuser.focuser.position == 125);
    assert(!focuser.focuser.moving);
    assert(!focuser.focuser.busy);
    assert(focuser.focuser.goto_rate == 2);
    const auto focuser_module = client.focuser(MountFocuserRequest{});
    assert(focuser_module.ok);

    const auto focuser_move_in = client.getFocuser(MountFocuserRequest{FocuserControlAction::MoveInward, 0});
    assert(focuser_move_in.ok);
    assert(mock_ptr->commands.back() == ":F-#");

    const auto focuser_move_out = client.getFocuser(MountFocuserRequest{FocuserControlAction::MoveOutward, 0});
    assert(focuser_move_out.ok);
    assert(mock_ptr->commands.back() == ":F+#");

    const auto focuser_stop = client.getFocuser(MountFocuserRequest{FocuserControlAction::Stop, 0});
    assert(focuser_stop.ok);
    assert(mock_ptr->commands.back() == ":FQ#");

    const auto focuser_goto = client.getFocuser(MountFocuserRequest{FocuserControlAction::GotoPosition, 250});
    assert(focuser_goto.ok);
    assert(mock_ptr->commands.back() == ":FS250#");

    const auto flip_mirror = client.getFlipMirror(MountFlipMirrorRequest{});
    assert(flip_mirror.ok);
    assert(flip_mirror.flip_mirror.position == FlipMirrorPosition::Camera);
    assert(mock_ptr->commands.back() == ":FM?#");

    const auto flip_mirror_module = client.flipMirror(MountFlipMirrorRequest{});
    assert(flip_mirror_module.ok);

    const bool camera_set = client.setFlipMirrorCamera();
    assert(camera_set);
    assert(mock_ptr->commands.back() == ":FM1#");
    const auto camera_state = client.getFlipMirrorState();
    assert(camera_state == FlipMirrorState::Camera);

    const bool eyepiece_set = client.setFlipMirrorEyepiece();
    assert(eyepiece_set);
    assert(mock_ptr->commands.back() == ":FM0#");
    const auto eyepiece_state = client.getFlipMirrorState();
    assert(eyepiece_state == FlipMirrorState::Eyepiece);

    auto invalid_flip_transport = std::make_unique<MockTransport>();
    invalid_flip_transport->invalid_responses.insert(":FM?#");
    OnStepMountClient invalid_flip_client(std::move(invalid_flip_transport));
    const auto invalid_flip = invalid_flip_client.getFlipMirror(MountFlipMirrorRequest{});
    assert(!invalid_flip.ok);
    assert(invalid_flip.error == MountError::InvalidResponse);

    const auto command_count_before = mock_ptr->commands.size();
    const auto flip_mirror_move = client.moveFlipMirror(MountFlipMirrorRequest{FlipMirrorPosition::Eyepiece});
    assert(flip_mirror_move.ok);
    assert(flip_mirror_move.flip_mirror.position == FlipMirrorPosition::Eyepiece);
    assert(std::find(mock_ptr->commands.begin(), mock_ptr->commands.end(), ":FM0#") != mock_ptr->commands.end());
    assert(mock_ptr->commands.back() == ":FM?#");
    assert(mock_ptr->commands.size() == command_count_before + 2);

    const auto guide = client.guide(MountGuideRequest{GuideDirection::North, 50, false});
    assert(guide.ok);

    const auto alignment = client.alignment(MountAlignmentRequest{MountAlignmentMode::Manual, 0});
    assert(alignment.ok);

    const auto tracking_rate = client.trackingRate(MountTrackingRateRequest{TrackingRatePreset::Lunar, 0.0, true});
    assert(tracking_rate.ok);
    assert(tracking_rate.rate == 0.75);

    const auto guide_rate = client.setGuideRate(MountGuideRateRequest{GuideRatePreset::Medium, 0.0, true});
    assert(guide_rate.ok);
    assert(guide_rate.rate == 0.5);

    const auto slew_rate = client.slewRate(MountSlewRateRequest{SlewRatePreset::Fast, 0.0, true});
    assert(slew_rate.ok);
    assert(slew_rate.rate == 1.5);

    const auto manual_motion = client.motion(MountManualMotionRequest{ManualMotionAxis::North, true, false});
    assert(manual_motion.ok);

    const auto aux = client.auxiliary(MountAuxRequest{true, false, false});
    assert(aux.ok);

    auto invalid_transport = std::make_unique<MockTransport>();
    invalid_transport->invalid_responses.insert(":Mgdn50#");
    invalid_transport->invalid_responses.insert(":GX90#");
    invalid_transport->invalid_responses.insert(":GX93#");
    OnStepMountClient invalid_client(std::move(invalid_transport));

    const auto invalid_guide = invalid_client.guide(MountGuideRequest{GuideDirection::North, 50, false});
    assert(!invalid_guide.ok);
    assert(invalid_guide.error == MountError::InvalidResponse);

    const auto invalid_guide_rate = invalid_client.setGuideRate(MountGuideRateRequest{GuideRatePreset::Medium, 0.0, true});
    assert(!invalid_guide_rate.ok);
    assert(invalid_guide_rate.error == MountError::InvalidResponse);

    const auto invalid_slew_rate = invalid_client.slewRate(MountSlewRateRequest{SlewRatePreset::Fast, 0.0, true});
    assert(!invalid_slew_rate.ok);
    assert(invalid_slew_rate.error == MountError::InvalidResponse);

    const auto home_move = client.getHomeStatus(MountHomeRequest{true});
    assert(home_move.ok);

    MountHomeConfigurationRequest home_config_request;
    home_config_request.query = true;
    home_config_request.enable_auto_home = true;
    home_config_request.apply_axis1_offset = true;
    home_config_request.axis1_offset_arcsec = 180;
    home_config_request.apply_axis2_offset = true;
    home_config_request.axis2_offset_arcsec = 240;
    home_config_request.save = true;
    const auto home_config = client.configureHome(home_config_request);
    assert(home_config.ok);
    assert(home_config.persisted);
    assert(home_config.configuration.valid);
    assert(home_config.configuration.auto_home_enabled);
    assert(home_config.configuration.axis1_offset_arcsec == 180);
    assert(home_config.configuration.axis2_offset_arcsec == 240);
    bool saw_auto_home = false;
    bool saw_axis1_offset = false;
    bool saw_axis2_offset = false;
    for (const auto& command : mock_ptr->commands) {
        if (command == ":hA1#") {
            saw_auto_home = true;
        } else if (command == ":hC1,180#") {
            saw_axis1_offset = true;
        } else if (command == ":hC2,240#") {
            saw_axis2_offset = true;
        }
    }
    assert(saw_auto_home);
    assert(saw_axis1_offset);
    assert(saw_axis2_offset);

    const auto park_action = client.getParkStatus(MountParkRequest{true});
    assert(park_action.ok);

    assert(client.isCapabilitySupported(MountCapabilityId::Site));
    assert(client.isCapabilitySupported(MountCapabilityId::Time));
    assert(client.isCapabilitySupported(MountCapabilityId::Pec));
    assert(client.isCapabilitySupported(MountCapabilityId::Limits));
    assert(client.isCapabilitySupported(MountCapabilityId::Focuser));
    assert(client.isCapabilitySupported(MountCapabilityId::FlipMirror));
    assert(client.isCapabilitySupported(MountCapabilityId::Orientation));
    assert(client.isCapabilitySupported(MountCapabilityId::Environmental));
    assert(client.isCapabilitySupported(MountCapabilityId::Rotator));

    const auto orientation = client.getOrientation(MountOrientationRequest{});
    assert(orientation.ok);
    assert(orientation.orientation.valid);
    assert(orientation.orientation.altitude_deg == 45.0);
    assert(orientation.orientation.azimuth_deg == 180.0);

    const auto environment = client.getEnvironment(MountEnvironmentRequest{});
    assert(environment.ok);
    assert(environment.environment.valid);
    assert(environment.environment.temperature_c == 12.3);
    assert(environment.environment.pressure_mb == 1013.2);
    assert(environment.environment.humidity_pct == 45.6);
    assert(environment.environment.dew_point_c == 5.0);
    assert(environment.environment.mcu_temperature_c == 60.0);

    const auto rotator = client.getRotator(MountRotatorRequest{RotatorControlAction::Query, true, true});
    assert(rotator.ok);
    assert(rotator.rotator.valid);
    assert(rotator.rotator.availability == RotatorAvailability::Available);
    assert(rotator.rotator.driver_status.has_value());

    // Get current angle
    const auto rot_angle = client.getRotator(MountRotatorRequest{RotatorControlAction::GetAngle, false, false});
    assert(rot_angle.ok);
    assert(rot_angle.rotator.current_angle_deg.has_value());
    assert(rot_angle.rotator.current_angle_deg.value() == 10.0);

    // Absolute goto (check exact command emitted)
    {
        MountRotatorRequest req;
        req.action = RotatorControlAction::AbsoluteGoto;
        req.target_angle_deg = 123.0;
        const auto resp = client.getRotator(req);
        assert(resp.ok);
        assert(mock_ptr->commands.back() == ":rS+123*00:00#");
    }

    // Relative rotate
    {
        MountRotatorRequest req; req.action = RotatorControlAction::Relative; req.relative_angle_deg = 5.0;
        const auto resp = client.getRotator(req);
        assert(resp.ok);
        assert(mock_ptr->commands.back() == ":rr+5*00:00#");
    }

    // Continuous CW
    {
        const auto resp = client.getRotator(MountRotatorRequest{RotatorControlAction::ContinuousCW, false, false});
        assert(resp.ok);
        assert(mock_ptr->commands.back() == ":r>#");
    }

    // Continuous CCW
    {
        const auto resp = client.getRotator(MountRotatorRequest{RotatorControlAction::ContinuousCCW, false, false});
        assert(resp.ok);
        assert(mock_ptr->commands.back() == ":r<#");
    }

    // Stop
    {
        const auto resp = client.getRotator(MountRotatorRequest{RotatorControlAction::Stop, false, false});
        assert(resp.ok);
        assert(mock_ptr->commands.back() == ":rQ#");
    }

    // Zero
    {
        const auto resp = client.getRotator(MountRotatorRequest{RotatorControlAction::SetZero, false, false});
        assert(resp.ok);
        assert(mock_ptr->commands.back() == ":rZ#");
    }

    // Set rate
    {
        MountRotatorRequest req; req.action = RotatorControlAction::SetRate; req.rate_index = 3;
        const auto resp = client.getRotator(req);
        assert(resp.ok);
        assert(mock_ptr->commands.back() == ":r3#");
    }

    // Derotate enable/disable/reverse
    {
        const auto e = client.getRotator(MountRotatorRequest{RotatorControlAction::DerotateEnable, false, false});
        assert(e.ok);
        assert(mock_ptr->commands.back() == ":r+#");
    }
    {
        const auto d = client.getRotator(MountRotatorRequest{RotatorControlAction::DerotateDisable, false, false});
        assert(d.ok);
        assert(mock_ptr->commands.back() == ":r-#");
    }
    {
        const auto r = client.getRotator(MountRotatorRequest{RotatorControlAction::DerotateReverse, false, false});
        assert(r.ok);
        assert(mock_ptr->commands.back() == ":rR#");
    }

    // Park / Unpark (rotator-parking uses :hP/:hR)
    {
        const auto p = client.getRotator(MountRotatorRequest{RotatorControlAction::Park, false, false});
        assert(p.ok);
        assert(mock_ptr->commands.back() == ":hP#");
    }
    {
        const auto u = client.getRotator(MountRotatorRequest{RotatorControlAction::Unpark, false, false});
        assert(u.ok);
        assert(mock_ptr->commands.back() == ":hR#");
    }

    const auto mode = client.getMountMode(MountModeRequest{});
    assert(mode.ok);
    assert(mode.mode == MountMode::AltAz);

    client.updateDetectedMountMode();
    assert(client.getDetectedMountMode() == MountMode::AltAz);

    asdevlab::TelescopeCore core;
    core.setDesiredMountMode(MountMode::Equatorial);
    assert(core.getDesiredMountMode() == MountMode::Equatorial);
    assert(!core.isMountModeMatched());

    const auto set_mode = core.selectMountMode(MountModeRequest{MountMode::Equatorial, false, false});
    assert(set_mode.ok);
    assert(set_mode.mode == MountMode::Equatorial);
    assert(set_mode.message.find("firmware rebuild") != std::string::npos);
    assert(core.hasPendingMountModeSelection());
    bool found_set = false;
    for (const auto &c : mock_ptr->commands) {
        if (c.find(":SXEM") == 0) found_set = true;
    }
    assert(!found_set);

    std::cout << "onstepx adapter mock transport test passed\n";
    return 0;
}
