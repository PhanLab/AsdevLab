#pragma once

#include <string>

namespace asdevlab {

class ImageService {
public:
    std::string start_live_stack();
    std::string stop_live_stack();

private:
    bool live_stacking_ = false;
};

} // namespace asdevlab
