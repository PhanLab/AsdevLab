#include "asdevlab/services/camera_service.hpp"

#include <cstdlib>
#include <sstream>

namespace asdevlab {
namespace {

bool command_exists(const std::string& command) {
    return std::system(("command -v " + command + " >/dev/null 2>&1").c_str()) == 0;
}

} // namespace

std::string CameraService::open_camera(const std::string& camera_id) {
    active_camera_ = camera_id;
    if (camera_id.rfind("uvc:", 0) == 0) {
        if (command_exists("v4l2-ctl")) {
            std::ostringstream oss;
            oss << "CameraService: opened " << camera_id << " via UVC backend";
            return oss.str();
        }
        std::ostringstream oss;
        oss << "CameraService: opened " << camera_id << " (simulated; v4l2-ctl unavailable)";
        return oss.str();
    }

    if (camera_id.rfind("android", 0) == 0) {
        std::ostringstream oss;
        oss << "CameraService: opened " << camera_id << " via Android backend";
        return oss.str();
    }

    std::ostringstream oss;
    oss << "CameraService: opened " << camera_id << " (simulated)";
    return oss.str();
}

std::string CameraService::start_stream() {
    streaming_ = true;
    std::ostringstream oss;
    oss << "CameraService: start_stream for " << (active_camera_.empty() ? "default" : active_camera_);
    return oss.str();
}

std::string CameraService::stop_stream() {
    streaming_ = false;
    return "CameraService: stop_stream";
}

} // namespace asdevlab
