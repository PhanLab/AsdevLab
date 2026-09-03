#pragma once

#include <string>

namespace asdevlab {

class SafetyService;
class MotionService;
class CameraService;
class PlateSolverService;
class StateMachine;

struct SchedulerResult {
    bool solved = false;
    int attempts = 0;
    std::string message;
};

class SchedulerService {
public:
    SchedulerService(SafetyService& safety,
                     StateMachine& state_machine,
                     MotionService& motion_service,
                     CameraService& camera_service,
                     PlateSolverService& plate_solver_service);

    SchedulerResult gotoAndSolve(const std::string& target, int max_retries = 5);
    std::string queue_capture(const std::string& target, int exposure_seconds);
    std::string run_sequence(const std::string& sequence_name);

private:
    SafetyService& safety_;
    StateMachine& state_machine_;
    MotionService& motion_service_;
    CameraService& camera_service_;
    PlateSolverService& plate_solver_service_;
};

} // namespace asdevlab
