#include "asdevlab/services/image_service.hpp"

#include <cstdlib>
#include <sstream>

namespace asdevlab {

std::string ImageService::start_live_stack() {
    live_stacking_ = true;
    const char* backend = std::getenv("ASDEVLAB_IMAGE_BACKEND");
    if (backend != nullptr && std::string(backend) == "openlivestacker") {
        return "ImageService: start_live_stack via OpenLiveStacker";
    }
    return "ImageService: start_live_stack (simulated)";
}

std::string ImageService::stop_live_stack() {
    live_stacking_ = false;
    return "ImageService: stop_live_stack";
}

} // namespace asdevlab
