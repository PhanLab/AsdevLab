#include "asdevlab/state_machine.hpp"

namespace asdevlab {

StateMachine::StateMachine() = default;

bool StateMachine::requestTransition(SystemState next_state, Owner owner) {
    if (current_owner() != Owner::Unknown && current_owner() != owner) {
        return false;
    }

    if (state_ == SystemState::ERROR || state_ == SystemState::PARKING) {
        return false;
    }

    switch (next_state) {
        case SystemState::IDLE:
            state_ = next_state;
            owner_ = Owner::Unknown;
            return true;
        case SystemState::TRACKING:
            if (state_ == SystemState::SLEWING || state_ == SystemState::GUIDING || state_ == SystemState::FOCUSING || state_ == SystemState::IDLE) {
                state_ = next_state;
                owner_ = Owner::Unknown;
                return true;
            }
            return false;
        case SystemState::SLEWING:
            if (state_ == SystemState::TRACKING || state_ == SystemState::IDLE) {
                state_ = next_state;
                owner_ = Owner::GotoService;
                return true;
            }
            return false;
        case SystemState::PARKING:
            if (state_ == SystemState::TRACKING || state_ == SystemState::IDLE || state_ == SystemState::SLEWING) {
                state_ = next_state;
                owner_ = Owner::MotionService;
                return true;
            }
            return false;
        case SystemState::PARKED:
            if (state_ == SystemState::PARKING) {
                state_ = next_state;
                owner_ = Owner::MotionService;
                return true;
            }
            return false;
        case SystemState::GUIDING:
            if (state_ == SystemState::TRACKING) {
                state_ = next_state;
                owner_ = Owner::GuideService;
                return true;
            }
            return false;
        case SystemState::FOCUSING:
            if (state_ == SystemState::TRACKING) {
                state_ = next_state;
                owner_ = Owner::FocusService;
                return true;
            }
            return false;
        default:
            return false;
    }
}

SystemState StateMachine::current_state() const {
    return state_;
}

Owner StateMachine::current_owner() const {
    return owner_;
}

} // namespace asdevlab
