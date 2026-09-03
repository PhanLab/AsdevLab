#pragma once

#include "asdevlab/hardware/mount/lx200_socket_transport.hpp"
#include "asdevlab/hardware/mount/mount_config.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace asdevlab {
namespace hardware {

class Lx200ConnectionManager {
public:
    enum class ConnectionStatus {
        Stopped,
        Disconnected,
        Connecting,
        Connected,
        Reconnecting
    };

    struct Callbacks {
        std::function<void()> onConnected;
        std::function<void()> onDisconnected;
        std::function<void()> onTimeout;
        std::function<void()> onReconnect;
    };

    explicit Lx200ConnectionManager(MountConfig config);
    explicit Lx200ConnectionManager(std::string host = "",
                                     int port = 0,
                                     int timeout_seconds = 0,
                                     int heartbeat_interval_seconds = 0);

    ~Lx200ConnectionManager();

    bool start();
    void stop();
    bool connect();
    void disconnect();
    bool isConnected() const;
    bool isRunning() const;
    ConnectionStatus status() const;
    void setCallbacks(Callbacks callbacks);
    void setTimeout(int timeout_seconds);
    void setHeartbeatInterval(int heartbeat_interval_seconds);
    bool sendCommand(const std::string& command, std::string& response_out);

private:
    void runLoop();
    bool attemptConnect();
    void updateStatus(ConnectionStatus status);
    void notifyCallbacks(const std::function<void()>& callback) const;

    mutable std::mutex mutex_;
    std::thread worker_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stop_requested_{false};
    std::unique_ptr<Lx200SocketTransport> transport_;
    std::string host_;
    int port_;
    int timeout_seconds_;
    int heartbeat_interval_seconds_;
    ConnectionStatus status_;
    Callbacks callbacks_;
};

} // namespace hardware
} // namespace asdevlab
