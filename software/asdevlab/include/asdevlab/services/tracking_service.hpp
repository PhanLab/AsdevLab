#pragma once

#include <string>

namespace asdevlab {

class TrackingService {
public:
    std::string set_mode(const std::string& mode);
    std::string start();
    std::string stop();
};

} // namespace asdevlab
