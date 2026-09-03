#include "asdevlab/hardware/mount/lx200_connection_manager.hpp"

#include <chrono>
#include <thread>

namespace asdevlab {
namespace hardware {

Lx200ConnectionManager::Lx200ConnectionManager(MountConfig config)
    : host_(config.host)
    , port_(config.port)
    , timeout_seconds_(config.timeout_seconds)
    , heartbeat_interval_seconds_(config.heartbeat_interval_seconds)
    , status_(ConnectionStatus::Stopped) {
    transport_ = std::make_unique<Lx200SocketTransport>(std::move(config));
}

Lx200ConnectionManager::Lx200ConnectionManager(std::string host,
                                                 int port,
                                                 int timeout_seconds,
                                                 int heartbeat_interval_seconds)
    : host_(std::move(host))
    , port_(port)
    , timeout_seconds_(timeout_seconds)
    , heartbeat_interval_seconds_(heartbeat_interval_seconds)
    , status_(ConnectionStatus::Stopped) {
    MountConfig config = MountConfig::fromEnvironment();
    if (!host_.empty()) {
        config.host = host_;
    }
    if (port != 0) {
        config.port = port_;
    }
    if (timeout_seconds != 0) {
        config.timeout_seconds = timeout_seconds_;
    }
    if (heartbeat_interval_seconds != 0) {
        config.heartbeat_interval_seconds = heartbeat_interval_seconds_;
    }
    transport_ = std::make_unique<Lx200SocketTransport>(config);
    host_ = config.host;
    port_ = config.port;
    timeout_seconds_ = config.timeout_seconds;
    heartbeat_interval_seconds_ = config.heartbeat_interval_seconds;
}

Lx200ConnectionManager::~Lx200ConnectionManager() {
    stop();
    if (worker_.joinable()) {
        worker_.join();
    }
}

bool Lx200ConnectionManager::start() {
    if (running_.load()) {
        return true;
    }

    stop_requested_.store(false);
    running_.store(true);
    worker_ = std::thread(&Lx200ConnectionManager::runLoop, this);
    return true;
}

void Lx200ConnectionManager::stop() {
    stop_requested_.store(true);
    running_.store(false);
    if (transport_) {
        transport_->disconnect();
    }
    if (worker_.joinable()) {
        worker_.join();
    }
}

bool Lx200ConnectionManager::connect() {
    if (!running_.load()) {
        start();
    }
    if (!attemptConnect()) {
        return false;
    }
    notifyCallbacks(callbacks_.onConnected);
    return true;
}

void Lx200ConnectionManager::disconnect() {
    if (transport_) {
        transport_->disconnect();
    }
    updateStatus(ConnectionStatus::Disconnected);
    notifyCallbacks(callbacks_.onDisconnected);
}

bool Lx200ConnectionManager::isConnected() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return transport_ && transport_->isConnected();
}

bool Lx200ConnectionManager::isRunning() const {
    return running_.load();
}

Lx200ConnectionManager::ConnectionStatus Lx200ConnectionManager::status() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return status_;
}

void Lx200ConnectionManager::setCallbacks(Callbacks callbacks) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_ = std::move(callbacks);
}

void Lx200ConnectionManager::setTimeout(int timeout_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    timeout_seconds_ = timeout_seconds;
    transport_ = std::make_unique<Lx200SocketTransport>(MountConfig{host_, port_, timeout_seconds_, heartbeat_interval_seconds_});
}

void Lx200ConnectionManager::setHeartbeatInterval(int heartbeat_interval_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    heartbeat_interval_seconds_ = heartbeat_interval_seconds;
}

bool Lx200ConnectionManager::sendCommand(const std::string& command, std::string& response_out) {
    if (!running_.load()) {
        start();
    }
    if (!transport_ || !transport_->isConnected()) {
        if (!attemptConnect()) {
            return false;
        }
    }

    if (!transport_ || !transport_->send(command, response_out)) {
        if (transport_) {
            transport_->disconnect();
        }
        updateStatus(ConnectionStatus::Disconnected);
        notifyCallbacks(callbacks_.onDisconnected);
        notifyCallbacks(callbacks_.onTimeout);
        return false;
    }

    return true;
}

void Lx200ConnectionManager::runLoop() {
    while (!stop_requested_.load()) {
        if (!transport_ || !transport_->isConnected()) {
            updateStatus(ConnectionStatus::Reconnecting);
            notifyCallbacks(callbacks_.onReconnect);
            if (!attemptConnect()) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
        }

        std::string response;
        if (!transport_ || !transport_->send(":GU#", response)) {
            if (transport_) {
                transport_->disconnect();
            }
            updateStatus(ConnectionStatus::Disconnected);
            notifyCallbacks(callbacks_.onDisconnected);
            notifyCallbacks(callbacks_.onTimeout);
        } else {
            updateStatus(ConnectionStatus::Connected);
            notifyCallbacks(callbacks_.onConnected);
        }

        for (int i = 0; i < heartbeat_interval_seconds_ && !stop_requested_.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

bool Lx200ConnectionManager::attemptConnect() {
    if (transport_ && transport_->isConnected()) {
        return true;
    }

    updateStatus(ConnectionStatus::Connecting);
    if (!transport_ || !transport_->connect(host_, port_)) {
        updateStatus(ConnectionStatus::Disconnected);
        notifyCallbacks(callbacks_.onDisconnected);
        return false;
    }

    updateStatus(ConnectionStatus::Connected);
    notifyCallbacks(callbacks_.onConnected);
    return true;
}

void Lx200ConnectionManager::updateStatus(ConnectionStatus status) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (status_ == status) {
        return;
    }
    status_ = status;
}

void Lx200ConnectionManager::notifyCallbacks(const std::function<void()>& callback) const {
    if (callback) {
        callback();
    }
}

} // namespace hardware
} // namespace asdevlab
