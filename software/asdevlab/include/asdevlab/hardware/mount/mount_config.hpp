#pragma once

#include <cstdlib>
#include <string>

namespace asdevlab {
namespace hardware {

struct MountConfig {
    std::string host{"192.168.0.1"};
    int port{9998};
    int timeout_seconds{2};
    int heartbeat_interval_seconds{5};

    static MountConfig fromEnvironment() {
        MountConfig config;
        if (const char* host = std::getenv("ASDEVLAB_ONSTEP_HOST")) {
            config.host = host;
        } else if (const char* host = std::getenv("ASDEVLAB_MOUNT_HOST")) {
            config.host = host;
        }
        if (const char* port = std::getenv("ASDEVLAB_ONSTEP_PORT")) {
            config.port = std::atoi(port);
        } else if (const char* port = std::getenv("ASDEVLAB_MOUNT_PORT")) {
            config.port = std::atoi(port);
        }
        if (const char* timeout = std::getenv("ASDEVLAB_ONSTEP_TIMEOUT_SECONDS")) {
            config.timeout_seconds = std::atoi(timeout);
        } else if (const char* timeout = std::getenv("ASDEVLAB_MOUNT_TIMEOUT_SECONDS")) {
            config.timeout_seconds = std::atoi(timeout);
        }
        if (const char* heartbeat = std::getenv("ASDEVLAB_ONSTEP_HEARTBEAT_SECONDS")) {
            config.heartbeat_interval_seconds = std::atoi(heartbeat);
        } else if (const char* heartbeat = std::getenv("ASDEVLAB_MOUNT_HEARTBEAT_SECONDS")) {
            config.heartbeat_interval_seconds = std::atoi(heartbeat);
        }
        return config;
    }
};

} // namespace hardware
} // namespace asdevlab
