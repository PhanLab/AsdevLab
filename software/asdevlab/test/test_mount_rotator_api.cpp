#include "asdevlab/web/mount_api.hpp"
#include "asdevlab/telescope_core.hpp"

#include <cassert>
#include <iostream>

int main() {
    asdevlab::TelescopeCore core;

    // call rotator JSON helper
    {
        auto j = asdevlab::web::mount_rotator_json(core, false);
        assert(j.find("\"ok\":") != std::string::npos);
    }

    std::cout << "mount rotator api smoke tests passed\n";
    return 0;
}
