#include "asdevlab/web/mount_api.hpp"
#include "asdevlab/telescope_core.hpp"

#include <cassert>
#include <iostream>

int main() {
    asdevlab::TelescopeCore core;

    // Basic smoke tests: ensure JSON helpers return valid JSON wrapper
    const auto status_json = asdevlab::web::mount_status_json(core, false);
    assert(status_json.find("\"ok\":") != std::string::npos);

    const auto coords_json = asdevlab::web::mount_coordinates_json(core, false);
    assert(coords_json.find("\"ok\":") != std::string::npos);

    const auto mode_json = asdevlab::web::mount_mode_json(core);
    assert(mode_json.find("\"ok\":") != std::string::npos);

    std::cout << "mount api smoke tests passed\n";
    return 0;
}
