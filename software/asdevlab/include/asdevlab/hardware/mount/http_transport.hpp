#pragma once

#include "asdevlab/hardware/mount/transport_interface.hpp"

#include <string>
#include <vector>

namespace asdevlab {
namespace hardware {

class HttpTransport : public TransportInterface {
public:
    explicit HttpTransport(std::string base_url, long timeout_seconds = 5);
    ~HttpTransport() override = default;

    bool send(const std::string& command, std::string& response_out) override;
    bool sendGet(const std::string& resource,
                 const std::vector<std::pair<std::string, std::string>>& query_params,
                 std::string& response_out) override;

private:
    std::string base_url_;
    long timeout_seconds_;
};

} // namespace hardware
} // namespace asdevlab
