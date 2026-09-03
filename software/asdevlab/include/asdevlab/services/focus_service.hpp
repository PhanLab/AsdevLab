#pragma once

#include <string>

namespace asdevlab {

class SafetyService;
class StateMachine;

class FocusService {
public:
    FocusService(SafetyService& safety, StateMachine& state_machine);
    std::string autofocus(const std::string& target, int exposure_ms);
    std::string manual_focus(int steps);

private:
    SafetyService& safety_;
    StateMachine& state_machine_;
};

} // namespace asdevlab
