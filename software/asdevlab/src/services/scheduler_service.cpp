#include "asdevlab/safety_service.hpp"
#include "asdevlab/services/scheduler_service.hpp"
#include "asdevlab/services/motion_service.hpp"
#include "asdevlab/services/camera_service.hpp"
#include "asdevlab/services/plate_solver_service.hpp"
#include "asdevlab/state_machine.hpp"

#include <cmath>
#include <sstream>

namespace asdevlab {

namespace {

std::optional<double> parse_offset(const std::string& output) {
    const std::string token = "offset=";
    const auto pos = output.find(token);
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    const auto start = pos + token.size();
    auto end = start;
    while (end < output.size() && (std::isdigit(static_cast<unsigned char>(output[end])) || output[end] == '.' || output[end] == '+' || output[end] == '-' || output[end] == 'e' || output[end] == 'E')) {
        end++;
    }
    if (end == start) {
        return std::nullopt;
    }
    try {
        return std::stod(output.substr(start, end - start));
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace

SchedulerService::SchedulerService(SafetyService& safety, StateMachine& state_machine, MotionService& motion_service, CameraService& camera_service, PlateSolverService& plate_solver_service)
    : safety_(safety)
    , state_machine_(state_machine)
    , motion_service_(motion_service)
    , camera_service_(camera_service)
    , plate_solver_service_(plate_solver_service)
{}

SchedulerResult SchedulerService::gotoAndSolve(const std::string& target, int max_retries) {
    SchedulerResult result;
    result.attempts = 0;
    for (int attempt = 1; attempt <= max_retries; ++attempt) {
        result.attempts = attempt;
        const auto goto_result = motion_service_.goto_target(target);
        if (goto_result.find("blocked by safety") != std::string::npos || goto_result.find("ERROR") != std::string::npos) {
            result.message = goto_result;
            if (attempt < max_retries) {
                continue;
            }
            state_machine_.requestTransition(SystemState::TRACKING, Owner::GotoService);
            return result;
        }

        camera_service_.open_camera("sim:0");
        const auto solve_result = plate_solver_service_.solve_result("frame_0000");
        if (solve_result.offset && *solve_result.offset <= 0.5) {
            std::ostringstream oss;
            oss << "PlateSolverService: solved image_id=frame_0000"
                << " target_ra=" << *solve_result.target_ra
                << " target_dec=" << *solve_result.target_dec
                << " mount_ra=" << *solve_result.mount_ra
                << " mount_dec=" << *solve_result.mount_dec
                << " offset=" << *solve_result.offset;
            if (solve_result.warning_message) {
                if (solve_result.warning_type) {
                    oss << " warning_type=" << *solve_result.warning_type;
                }
                oss << " warning='" << *solve_result.warning_message << "'";
            }
            result.message = oss.str();
            if (attempt < max_retries) {
                continue;
            }
            result.solved = false;
            state_machine_.requestTransition(SystemState::TRACKING, Owner::GotoService);
            return result;
        }

        std::ostringstream oss;
        oss << "PlateSolverService: solve frame_0000";
        if (solve_result.warning_message) {
            if (solve_result.warning_type) {
                oss << " warning_type=" << *solve_result.warning_type;
            }
            oss << " warning='" << *solve_result.warning_message << "'";
        }
        result.message = oss.str();
        if (attempt < max_retries) {
            // Retry by attempting another goto.
            continue;
        }
    }

    state_machine_.requestTransition(SystemState::TRACKING, Owner::GotoService);
    return result;
}

std::string SchedulerService::queue_capture(const std::string& target, int exposure_seconds) {
    if (!state_machine_.requestTransition(SystemState::SLEWING, Owner::GotoService)) {
        return "SchedulerService: system busy";
    }

    if (!safety_.can_send_motion_command("goto")) {
        state_machine_.requestTransition(SystemState::TRACKING, Owner::GotoService);
        return "SchedulerService: blocked by safety: " + safety_.reason();
    }

    std::ostringstream oss;
    oss << "SchedulerService: queue_capture target=" << target << " exposure=" << exposure_seconds;
    return oss.str();
}

std::string SchedulerService::run_sequence(const std::string& sequence_name) {
    if (!safety_.can_send_motion_command("sequence")) {
        return "SchedulerService: blocked by safety: " + safety_.reason();
    }

    std::ostringstream oss;
    oss << "SchedulerService: run_sequence " << sequence_name;
    return oss.str();
}

} // namespace asdevlab
