#pragma once

#include <optional>
#include <string>

namespace asdevlab {

struct EquatorialCoord {
    double ra_hours;
    double dec_degrees;
};

class SafetyService {
public:
    enum class SystemState {
        Normal,
        Focusing,
        Tracking,
        Slewing
    };

    SafetyService();

    bool can_send_motion_command(const std::string& command,
                                 std::optional<EquatorialCoord> target = std::nullopt) const;
    std::string reason() const;
    void set_system_state(SystemState state);
    SystemState system_state() const;

private:
    mutable std::string last_reason_;
    SystemState system_state_;
};

} // namespace asdevlab
