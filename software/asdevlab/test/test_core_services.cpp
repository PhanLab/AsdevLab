#include "asdevlab/safety_service.hpp"
#include "asdevlab/telescope_core.hpp"

#include <cassert>
#include <iostream>

static asdevlab::EquatorialCoord high_altitude_target() {
    return asdevlab::EquatorialCoord{0.0, 89.0};
}

static asdevlab::EquatorialCoord low_altitude_target() {
    return asdevlab::EquatorialCoord{0.0, -90.0};
}

int main() {
    asdevlab::TelescopeCore core;
    asdevlab::SafetyService& safety = core.safety();

    // Smoke test for motion, camera, focus services.
    const auto park = core.motion().park();
    assert(park.find("simulated") != std::string::npos || park.find("onstep") != std::string::npos);

    const auto camera = core.camera().open_camera("uvc:0");
    assert(camera.find("CameraService") != std::string::npos);

    const auto focus = core.focus().autofocus("M31", 200);
    assert(focus.find("FocusService") != std::string::npos);

    // Safety logic tests.
    assert(safety.can_send_motion_command("goto", high_altitude_target()));
    assert(safety.reason().empty());
    assert(!safety.can_send_motion_command("goto", low_altitude_target()));
    assert(safety.reason().find("below minimum altitude") != std::string::npos);

    safety.set_system_state(asdevlab::SafetyService::SystemState::Focusing);
    assert(!safety.can_send_motion_command("goto", high_altitude_target()));
    assert(safety.reason().find("blocked while focusing") != std::string::npos);

    assert(safety.can_send_motion_command("park"));
    assert(safety.can_send_motion_command("stop"));

    const auto first_capture = core.scheduler().queue_capture("M31", 10);
    assert(first_capture.find("SchedulerService") != std::string::npos);
    const auto guide_blocked = core.guide().guide_step(1, 1);
    assert(guide_blocked.find("system busy") != std::string::npos);

    std::cout << "asdevlab service smoke test passed\n";
    return 0;
}
