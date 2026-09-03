#include "asdevlab/safety_service.hpp"
#include "asdevlab/services/guide_service.hpp"
#include "asdevlab/state_machine.hpp"

#include <sstream>

namespace asdevlab {

GuideService::GuideService(SafetyService& safety, StateMachine& state_machine)
    : safety_(safety)
    , state_machine_(state_machine)
{}

std::string GuideService::guide_step(int dx, int dy) {
    if (!state_machine_.requestTransition(SystemState::GUIDING, Owner::GuideService)) {
        return "GuideService: system busy";
    }

    if (!safety_.can_send_motion_command("guide")) {
        state_machine_.requestTransition(SystemState::TRACKING, Owner::GuideService);
        return "GuideService: blocked by safety: " + safety_.reason();
    }

    std::ostringstream oss;
    oss << "GuideService: guide_step dx=" << dx << " dy=" << dy;
    return oss.str();
}

} // namespace asdevlab
