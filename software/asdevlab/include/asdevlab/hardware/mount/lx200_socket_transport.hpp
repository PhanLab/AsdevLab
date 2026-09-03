#pragma once

#include "asdevlab/hardware/mount/mount_config.hpp"
#include "asdevlab/hardware/mount/transport_interface.hpp"

#include <mutex>
#include <string>
#include <vector>

namespace asdevlab {
namespace hardware {

class Lx200SocketTransport : public TransportInterface {
public:
    explicit Lx200SocketTransport(MountConfig config);
    explicit Lx200SocketTransport(std::string host,
                                  int port,
                                  int timeout_seconds);

    bool connect(const std::string& host, int port);
    void disconnect();
    bool isConnected() const;

    bool send(const std::string& command, std::string& response_out) override;
    bool sendGet(const std::string& resource,
                 const std::vector<std::pair<std::string, std::string>>& query_params,
                 std::string& response_out) override;

private:
    bool ensureConnected();
    bool ensureConnectedLocked();
    bool connectLocked(const std::string& host, int port);
    bool writeAll(const std::string& data);
    bool writeAllLocked(const std::string& data);
    bool readUntilTerminator(std::string& response_out);
    bool readUntilTerminatorLocked(std::string& response_out);
    void disconnectLocked();
    void setSocketTimeouts();

    std::string host_;
    int port_;
    int timeout_seconds_;
    int socket_fd_;
    bool connected_;
    mutable std::mutex mutex_;
};

} // namespace hardware
} // namespace asdevlab
