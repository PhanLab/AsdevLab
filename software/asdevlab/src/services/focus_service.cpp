#include "asdevlab/services/focus_service.hpp"
#include "asdevlab/state_machine.hpp"

#include <sstream>

namespace asdevlab {

FocusService::FocusService(SafetyService& safety, StateMachine& state_machine)
    : safety_(safety)
    , state_machine_(state_machine)
{}

std::string FocusService::autofocus(const std::string& target, int exposure_ms) {
    if (!state_machine_.requestTransition(SystemState::FOCUSING, Owner::FocusService)) {
        return "FocusService: system busy";
    }

    std::ostringstream oss;
    oss << "FocusService: autofocus target=" << target << " exposure_ms=" << exposure_ms
        << " (simulated hfr/contrast loop)";
    state_machine_.requestTransition(SystemState::TRACKING, Owner::FocusService);
    return oss.str();
}

std::string FocusService::manual_focus(int steps) {
    std::ostringstream oss;
    oss << "FocusService: manual_focus steps=" << steps;
    return oss.str();
}

} // namespace asdevlab
