#pragma once

#include <string>

namespace asdevlab {

class CameraService {
public:
    std::string open_camera(const std::string& camera_id);
    std::string start_stream();
    std::string stop_stream();

private:
    std::string active_camera_;
    bool streaming_ = false;
};

} // namespace asdevlab
