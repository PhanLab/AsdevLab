#include "asdevlab/hardware/mount/lx200_connection_manager.hpp"
#include "asdevlab/hardware/mount/mount_config.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

int main() {
    const char* run_flag = std::getenv("ASDEVLAB_RUN_MOUNT_INTEGRATION_TESTS");
    const char* run_host = std::getenv("ASDEVLAB_ONSTEP_HOST");
    if (!run_host) {
        run_host = std::getenv("ASDEVLAB_MOUNT_HOST");
    }
    const char* run_port = std::getenv("ASDEVLAB_ONSTEP_PORT");
    if (!run_port) {
        run_port = std::getenv("ASDEVLAB_MOUNT_PORT");
    }

    if (!run_flag || std::string(run_flag) != "1") {
        std::cout << "mount integration test skipped; set ASDEVLAB_RUN_MOUNT_INTEGRATION_TESTS=1 and ASDEVLAB_ONSTEP_HOST to enable\n";
        return 0;
    }

    if (!run_host || std::string(run_host).empty() || !run_port || std::string(run_port).empty()) {
        std::cerr << "mount integration test requires ASDEVLAB_ONSTEP_HOST (or ASDEVLAB_MOUNT_HOST) and ASDEVLAB_ONSTEP_PORT (or ASDEVLAB_MOUNT_PORT) when enabled\n";
        return 1;
    }

    auto config = asdevlab::hardware::MountConfig::fromEnvironment();
    config.host = run_host;
    config.port = std::atoi(run_port);

    asdevlab::hardware::Lx200ConnectionManager manager(config);
    if (!manager.start()) {
        std::cerr << "failed to start mount connection manager\n";
        return 1;
    }

    std::string response;
    if (!manager.sendCommand(":GU#", response)) {
        manager.stop();
        std::cerr << "mount integration test failed to receive a response from the mount\n";
        return 1;
    }

    if (response.empty()) {
        manager.stop();
        std::cerr << "mount integration test received an empty response\n";
        return 1;
    }

    std::cout << "mount integration test connected successfully and received: " << response << "\n";
    manager.stop();
    return 0;
}
