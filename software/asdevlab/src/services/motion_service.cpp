#include "asdevlab/hardware/mount/mount_interface.hpp"
#include "asdevlab/safety_service.hpp"
#include "asdevlab/services/motion_service.hpp"
#include "asdevlab/state_machine.hpp"

#include <cctype>
#include <string>
#include <vector>

namespace asdevlab {
namespace {

bool is_query_safe(const std::string& query) {
    for (unsigned char ch : query) {
        if (std::isalnum(ch) || ch == '_' || ch == '.' || ch == '-' || ch == '+' || ch == '=' || ch == '&' || ch == ' ') {
            continue;
        }
        return false;
    }
    return true;
}

} // namespace

MotionService::MotionService(SafetyService& safety, StateMachine& state_machine, hardware::MountInterface& mount, ObservationService& observation_service, astronomy::AstronomyService& astronomy_service)
    : safety_(safety)
    , state_machine_(state_machine)
    , mount_(mount)
    , observation_service_(observation_service)
    , astronomy_service_(astronomy_service)
{}

bool MotionService::is_allowed(const std::string& command) const {
    return safety_.can_send_motion_command(command);
}

hardware::MountMode MotionService::getMountMode() const {
    const auto mode_response = mount_.getMountMode(hardware::MountModeRequest{});
    if (mode_response.ok && mode_response.supported) {
        return mode_response.mode;
    }
    return hardware::MountMode::Equatorial;
}

MotionCommandResult MotionService::goto_target(const catalog::ResolvedTarget& target, std::string& log) {
    if (!target.resolved) {
        log = "MotionService: target not resolved";
        return {false, log};
    }

    const auto context = observation_service_.makeObservationContext();
    const auto mount_mode = getMountMode();
    if (mount_mode == hardware::MountMode::AltAz) {
        auto resolved = astronomy_service_.computeHorizontalCoordinates(target, context);
        if (!resolved.horizontal_computed) {
            log = "MotionService: failed to compute horizontal coordinates";
            return {false, log};
        }
        return goto_target(resolved.ra_hours, resolved.dec_degrees, true, resolved.alt_degrees, resolved.az_degrees, log);
    }

    return goto_target(target.ra_hours, target.dec_degrees, log);
}

MotionCommandResult MotionService::sync(const catalog::ResolvedTarget& target, std::string& log) {
    if (!target.resolved) {
        log = "MotionService: target not resolved";
        return {false, log};
    }

    const auto context = observation_service_.makeObservationContext();
    const auto mount_mode = getMountMode();
    if (mount_mode == hardware::MountMode::AltAz) {
        auto resolved = astronomy_service_.computeHorizontalCoordinates(target, context);
        if (!resolved.horizontal_computed) {
            log = "MotionService: failed to compute horizontal coordinates";
            return {false, log};
        }
        return sync(resolved.ra_hours, resolved.dec_degrees, true, resolved.alt_degrees, resolved.az_degrees, log);
    }

    return sync(target.ra_hours, target.dec_degrees, log);
}

MotionCommandResult MotionService::goto_target(double ra_hours, double dec_deg, std::string& log) {
    std::vector<std::string> commands;
    return goto_target(ra_hours, dec_deg, false, 0.0, 0.0, false, commands, log);
}

MotionCommandResult MotionService::goto_target(double ra_hours, double dec_deg, bool use_alt_az, double alt_deg, double az_deg, std::string& log) {
    std::vector<std::string> commands;
    return goto_target(ra_hours, dec_deg, use_alt_az, alt_deg, az_deg, false, commands, log);
}

MotionCommandResult MotionService::goto_target(double ra_hours,
                                             double dec_deg,
                                             bool use_alt_az,
                                             double alt_deg,
                                             double az_deg,
                                             bool dry_run,
                                             std::vector<std::string>& commands_out,
                                             std::string& log) {
    if (!is_allowed("goto")) {
        log = "MotionService: blocked by safety: " + safety_.reason();
        return {false, log};
    }

    hardware::MountGotoRequest request{ra_hours, dec_deg, use_alt_az, alt_deg, az_deg};

    if (dry_run) {
        // MotionService does not format protocol commands. Dry-run/preview
        // functionality is a CLI/debug concern and should query mount
        // implementations (e.g. OnStepMountClient) directly. Here we
        // short-circuit to indicate dry-run was accepted without sending.
        log = "MotionService: dry-run acknowledged (no transport)";
        return {true, log};
    }

    if (!state_machine_.requestTransition(SystemState::SLEWING, Owner::GotoService)) {
        log = "MotionService: system busy";
        return {false, log};
    }

    const auto goto_response = mount_.gotoTarget(request);
    if (!goto_response.ok || goto_response.error != hardware::MountError::None) {
        state_machine_.requestTransition(SystemState::IDLE, Owner::GotoService);
        log = "MotionService: mount goto failed";
        return {false, log};
    }

    const bool tracking_ok = mount_.startTracking();
    if (!tracking_ok) {
        state_machine_.requestTransition(SystemState::IDLE, Owner::GotoService);
        log = "MotionService: mount tracking failed after goto";
        return {false, log};
    }

    state_machine_.requestTransition(SystemState::TRACKING, Owner::GotoService);
    log = "MotionService: goto ok";
    return {true, log};
}

MotionCommandResult MotionService::sync(double ra_hours, double dec_deg, std::string& log) {
    return sync(ra_hours, dec_deg, false, 0.0, 0.0, log);
}

MotionCommandResult MotionService::sync(double ra_hours, double dec_deg, bool use_alt_az, double alt_deg, double az_deg, std::string& log) {
    if (!is_allowed("sync")) {
        log = "MotionService: blocked by safety: " + safety_.reason();
        return {false, log};
    }

    const auto sync_response = mount_.syncTarget(hardware::MountSyncRequest{ra_hours, dec_deg, use_alt_az, alt_deg, az_deg});
    if (!sync_response.ok || sync_response.error != hardware::MountError::None) {
        log = "MotionService: mount sync failed";
        return {false, log};
    }

    log = "MotionService: sync ok";
    return {true, log};
}

MotionCommandResult MotionService::start_tracking(std::string& log) {
    if (!is_allowed("tracking")) {
        log = "MotionService: blocked by safety: " + safety_.reason();
        return {false, log};
    }

    if (state_machine_.current_state() == SystemState::TRACKING) {
        log = "MotionService: tracking already active";
        return {true, log};
    }

    if (!state_machine_.requestTransition(SystemState::TRACKING, Owner::MotionService)) {
        log = "MotionService: transition denied";
        return {false, log};
    }

    const bool ok = mount_.startTracking();
    if (!ok) {
        state_machine_.requestTransition(SystemState::IDLE, Owner::MotionService);
        log = "MotionService: start tracking failed";
        return {false, log};
    }

    log = "MotionService: tracking started";
    return {true, log};
}

MotionCommandResult MotionService::stop_tracking(std::string& log) {
    if (!is_allowed("stop")) {
        log = "MotionService: blocked by safety: " + safety_.reason();
        return {false, log};
    }

    const bool ok = mount_.stopTracking();
    if (!ok) {
        log = "MotionService: stop tracking failed";
        return {false, log};
    }

    state_machine_.requestTransition(SystemState::IDLE, Owner::MotionService);
    log = "MotionService: tracking stopped";
    return {true, log};
}

MotionCommandResult MotionService::park(std::string& log) {
    if (!is_allowed("park")) {
        log = "MotionService: blocked by safety: " + safety_.reason();
        return {false, log};
    }

    if (!state_machine_.requestTransition(SystemState::PARKING, Owner::MotionService)) {
        log = "MotionService: transition denied";
        return {false, log};
    }

    const bool ok = mount_.park();
    if (!ok) {
        state_machine_.requestTransition(SystemState::IDLE, Owner::MotionService);
        log = "MotionService: park failed";
        return {false, log};
    }

    state_machine_.requestTransition(SystemState::PARKED, Owner::MotionService);
    log = "MotionService: parked";
    return {true, log};
}

MotionCommandResult MotionService::home(std::string& log) {
    if (!is_allowed("home")) {
        log = "MotionService: blocked by safety: " + safety_.reason();
        return {false, log};
    }

    const bool ok = mount_.home();
    if (!ok) {
        log = "MotionService: home failed";
        return {false, log};
    }

    log = "MotionService: home ok";
    return {true, log};
}

MotionCommandResult MotionService::abort(std::string& log) {
    const bool ok = mount_.abort();
    if (!ok) {
        log = "MotionService: abort failed";
        return {false, log};
    }

    state_machine_.requestTransition(SystemState::IDLE, Owner::MotionService);
    log = "MotionService: aborted";
    return {true, log};
}

MotionCommandResult MotionService::status(std::string& log) {
    const std::string status = mount_.getStatus();
    log = "MotionService: " + status;
    return {true, log};
}

std::string MotionService::goto_target(const std::string& target) {
    if (!is_query_safe(target)) {
        return "MotionService: rejected invalid query characters";
    }

    return "MotionService: goto accepted; target resolution is handled by TargetService";
}

std::string MotionService::move(const std::string& direction, int steps) {
    std::string normalized;
    normalized.reserve(direction.size());
    for (unsigned char ch : direction) {
        normalized.push_back(static_cast<char>(std::tolower(ch)));
    }

    hardware::MountManualMotionRequest request;
    if (normalized == "north") {
        request.axis = hardware::ManualMotionAxis::North;
        request.start = true;
    } else if (normalized == "south") {
        request.axis = hardware::ManualMotionAxis::South;
        request.start = true;
    } else if (normalized == "east") {
        request.axis = hardware::ManualMotionAxis::East;
        request.start = true;
    } else if (normalized == "west") {
        request.axis = hardware::ManualMotionAxis::West;
        request.start = true;
    } else if (normalized == "stop") {
        request.axis = hardware::ManualMotionAxis::None;
        request.start = false;
    } else {
        return "MotionService: invalid move direction";
    }

    if (request.start) {
        if (!is_allowed("move")) {
            return "MotionService: blocked by safety: " + safety_.reason();
        }
    } else {
        if (!is_allowed("stop")) {
            return "MotionService: blocked by safety: " + safety_.reason();
        }
    }

    const auto response = mount_.manualMotion(request);
    if (!response.ok) {
        return "MotionService: move failed";
    }
    return response.message;
}

std::string MotionService::park() {
    std::string log;
    const auto result = park(log);
    return result.ok ? result.log : (result.log.find("simulated") != std::string::npos ? result.log : "MotionService: park (simulated)");
}

std::string MotionService::stop() {
    std::string log;
    const auto result = stop_tracking(log);
    return result.ok ? result.log : "MotionService: stop (simulated)";
}

std::string MotionService::set_speed(double speed) {
    (void)speed;
    return "MotionService: speed control is not implemented in this mount adapter";
}

} // namespace asdevlab
