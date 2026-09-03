#pragma once

#include <string>

namespace asdevlab {

class SafetyService;
class StateMachine;

class GuideService {
public:
    GuideService(SafetyService& safety, StateMachine& state_machine);
    std::string guide_step(int dx, int dy);

private:
    SafetyService& safety_;
    StateMachine& state_machine_;
};

} // namespace asdevlab
