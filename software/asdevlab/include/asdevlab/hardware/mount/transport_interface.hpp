#pragma once

#include <string>
#include <vector>

namespace asdevlab {
namespace hardware {

class TransportInterface {
public:
    virtual ~TransportInterface() = default;

    // Send a raw LX200 command to the transport used by OnStepX and
    // return the raw response in `response_out`.
    // Returns true on success, false on failure.
    virtual bool send(const std::string& command, std::string& response_out) = 0;

    // Send a transport-specific GET request when a transport needs query-style
    // access in addition to raw command text. Implementations are free to ignore it.
    virtual bool sendGet(const std::string& resource,
                         const std::vector<std::pair<std::string, std::string>>& query_params,
                         std::string& response_out) = 0;
};

} // namespace hardware
} // namespace asdevlab
