#pragma once

#include "asdevlab/astronomy/astronomy_service.hpp"
#include "asdevlab/catalog/resolved_target.hpp"
#include "asdevlab/hardware/mount/mount_interface.hpp"
#include "asdevlab/services/observation_service.hpp"

#include <string>
#include <vector>

namespace asdevlab {

class SafetyService;
class StateMachine;

struct MotionCommandResult {
    bool ok;
    std::string log;
};

class MotionService {
public:
    // Current motion handling assumes a single active workflow and a single caller sequence.
    // TODO: If concurrent motion requests are allowed later, the shared mount/state-machine
    // transitions in this service will need protection alongside workflow state updates.
    MotionService(SafetyService& safety, StateMachine& state_machine, hardware::MountInterface& mount, ObservationService& observation_service, astronomy::AstronomyService& astronomy_service);

    hardware::MountMode getMountMode() const;

    MotionCommandResult goto_target(const catalog::ResolvedTarget& target, std::string& log);
    MotionCommandResult sync(const catalog::ResolvedTarget& target, std::string& log);

    MotionCommandResult goto_target(double ra_hours, double dec_deg, std::string& log);
    MotionCommandResult goto_target(double ra_hours, double dec_deg, bool use_alt_az, double alt_deg, double az_deg, std::string& log);
    MotionCommandResult goto_target(double ra_hours, double dec_deg, bool use_alt_az, double alt_deg, double az_deg, bool dry_run, std::vector<std::string>& commands_out, std::string& log);
    MotionCommandResult sync(double ra_hours, double dec_deg, std::string& log);
    MotionCommandResult sync(double ra_hours, double dec_deg, bool use_alt_az, double alt_deg, double az_deg, std::string& log);
    MotionCommandResult start_tracking(std::string& log);
    MotionCommandResult stop_tracking(std::string& log);
    MotionCommandResult park(std::string& log);
    MotionCommandResult home(std::string& log);
    MotionCommandResult abort(std::string& log);
    MotionCommandResult status(std::string& log);

    std::string goto_target(const std::string& target);
    std::string move(const std::string& direction, int steps);
    std::string park();
    std::string stop();
    std::string set_speed(double speed);

private:
    SafetyService& safety_;
    StateMachine& state_machine_;
    hardware::MountInterface& mount_;
    ObservationService& observation_service_;
    astronomy::AstronomyService& astronomy_service_;
    bool is_allowed(const std::string& command) const;
};

} // namespace asdevlab
