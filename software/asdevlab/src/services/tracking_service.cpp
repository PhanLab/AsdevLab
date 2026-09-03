#include "asdevlab/services/tracking_service.hpp"

#include <sstream>

namespace asdevlab {

std::string TrackingService::set_mode(const std::string& mode) {
    std::ostringstream oss;
    oss << "TrackingService: mode " << mode;
    return oss.str();
}

std::string TrackingService::start() {
    return "TrackingService: start";
}

std::string TrackingService::stop() {
    return "TrackingService: stop";
}

} // namespace asdevlab
