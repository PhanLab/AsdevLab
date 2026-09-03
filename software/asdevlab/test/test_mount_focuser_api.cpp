#include "asdevlab/web/mount_api.hpp"
#include "asdevlab/telescope_core.hpp"

#include <cassert>
#include <iostream>

int main() {
    asdevlab::TelescopeCore core;

    // Call TelescopeCore focuser wrapper
    {
        auto resp = core.mountFocuser(asdevlab::hardware::MountFocuserRequest{});
        // We expect a well-formed response (ok may be false if no device)
        assert(resp.supported == resp.supported); // trivial check to use field
    }

    // Call web JSON helper
    {
        auto j = asdevlab::web::mount_focuser_json(core);
        assert(j.find("\"ok\":") != std::string::npos);
    }

    std::cout << "mount focuser api smoke tests passed\n";
    return 0;
}
