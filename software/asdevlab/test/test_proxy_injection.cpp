#include "asdevlab/services/motion_service.hpp"
#include "asdevlab/telescope_core.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>

int main() {
    asdevlab::TelescopeCore core;
    auto& motion = core.motion();

    const auto unsafe_target = "'; rm -rf ~ #";
    const auto response = motion.goto_target(unsafe_target);
    assert(response.find("invalid query characters") != std::string::npos);

    const auto park_response = motion.park();
    assert(park_response.find("park") != std::string::npos);

    std::cout << "asdevlab proxy injection test passed\n";
    return 0;
}
