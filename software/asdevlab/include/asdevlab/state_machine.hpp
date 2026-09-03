#pragma once

namespace asdevlab {

enum class SystemState {
    IDLE,
    SLEWING,
    GUIDING,
    FOCUSING,
    TRACKING,
    ERROR,
    PARKING,
    PARKED
};

enum class Owner {
    Unknown,
    GotoService,
    GuideService,
    FocusService,
    SchedulerService,
    MotionService
};

class StateMachine {
public:
    StateMachine();

    bool requestTransition(SystemState next_state, Owner owner);
    SystemState current_state() const;
    Owner current_owner() const;

private:
    SystemState state_ = SystemState::IDLE;
    Owner owner_ = Owner::Unknown;
};

} // namespace asdevlab
