#include "asdevlab/hardware/mount/lx200_socket_transport.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace asdevlab {
namespace hardware {
namespace {

constexpr int kInvalidSocket = -1;

} // namespace

Lx200SocketTransport::Lx200SocketTransport(MountConfig config)
    : host_(std::move(config.host))
    , port_(config.port)
    , timeout_seconds_(config.timeout_seconds)
    , socket_fd_(kInvalidSocket)
    , connected_(false) {}

Lx200SocketTransport::Lx200SocketTransport(std::string host, int port, int timeout_seconds)
    : host_(std::move(host))
    , port_(port)
    , timeout_seconds_(timeout_seconds)
    , socket_fd_(kInvalidSocket)
    , connected_(false) {}

bool Lx200SocketTransport::connect(const std::string& host, int port) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (socket_fd_ != kInvalidSocket) {
        ::close(socket_fd_);
        socket_fd_ = kInvalidSocket;
    }
    connected_ = false;

    socket_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ == kInvalidSocket) {
        connected_ = false;
        return false;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(static_cast<uint16_t>(port));

    if (::inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) != 1) {
        ::close(socket_fd_);
        socket_fd_ = kInvalidSocket;
        connected_ = false;
        return false;
    }

    setSocketTimeouts();

    if (::connect(socket_fd_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) != 0) {
        ::close(socket_fd_);
        socket_fd_ = kInvalidSocket;
        connected_ = false;
        return false;
    }

    host_ = host;
    port_ = port;
    connected_ = true;
    return true;
}

void Lx200SocketTransport::disconnect() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (socket_fd_ != kInvalidSocket) {
        ::close(socket_fd_);
        socket_fd_ = kInvalidSocket;
    }
    connected_ = false;
}

bool Lx200SocketTransport::isConnected() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return connected_ && socket_fd_ != kInvalidSocket;
}

bool Lx200SocketTransport::send(const std::string& command, std::string& response_out) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!ensureConnectedLocked()) {
        return false;
    }

    const std::string framed_command = command.empty() ? "#" : (command.back() == '#' ? command : command + "#");
    if (!writeAllLocked(framed_command)) {
        disconnectLocked();
        return false;
    }

    response_out.clear();
    if (!readUntilTerminatorLocked(response_out)) {
        disconnectLocked();
        return false;
    }

    return true;
}

bool Lx200SocketTransport::sendGet(const std::string& resource,
                                   const std::vector<std::pair<std::string, std::string>>& query_params,
                                   std::string& response_out) {
    (void)resource;
    (void)query_params;
    response_out.clear();
    return false;
}

bool Lx200SocketTransport::ensureConnected() {
    std::lock_guard<std::mutex> lock(mutex_);
    return ensureConnectedLocked();
}

bool Lx200SocketTransport::ensureConnectedLocked() {
    if (connected_ && socket_fd_ != kInvalidSocket) {
        return true;
    }
    return connectLocked(host_, port_);
}

bool Lx200SocketTransport::connectLocked(const std::string& host, int port) {
    if (socket_fd_ != kInvalidSocket) {
        ::close(socket_fd_);
        socket_fd_ = kInvalidSocket;
    }
    connected_ = false;

    socket_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ == kInvalidSocket) {
        connected_ = false;
        return false;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(static_cast<uint16_t>(port));

    if (::inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) != 1) {
        ::close(socket_fd_);
        socket_fd_ = kInvalidSocket;
        connected_ = false;
        return false;
    }

    setSocketTimeouts();

    if (::connect(socket_fd_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) != 0) {
        ::close(socket_fd_);
        socket_fd_ = kInvalidSocket;
        connected_ = false;
        return false;
    }

    host_ = host;
    port_ = port;
    connected_ = true;
    return true;
}

bool Lx200SocketTransport::writeAll(const std::string& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    return writeAllLocked(data);
}

bool Lx200SocketTransport::writeAllLocked(const std::string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        const ssize_t written = ::send(socket_fd_, data.data() + sent, data.size() - sent, 0);
        if (written <= 0) {
            return false;
        }
        sent += static_cast<size_t>(written);
    }
    return true;
}

bool Lx200SocketTransport::readUntilTerminator(std::string& response_out) {
    std::lock_guard<std::mutex> lock(mutex_);
    return readUntilTerminatorLocked(response_out);
}

bool Lx200SocketTransport::readUntilTerminatorLocked(std::string& response_out) {
    char ch = '\0';
    while (true) {
        const ssize_t received = ::recv(socket_fd_, &ch, 1, 0);
        if (received <= 0) {
            return false;
        }
        if (ch == '#') {
            return true;
        }
        response_out.push_back(ch);
    }
}

void Lx200SocketTransport::disconnectLocked() {
    if (socket_fd_ != kInvalidSocket) {
        ::close(socket_fd_);
        socket_fd_ = kInvalidSocket;
    }
    connected_ = false;
}

void Lx200SocketTransport::setSocketTimeouts() {
    timeval timeout{};
    timeout.tv_sec = timeout_seconds_;
    timeout.tv_usec = 0;
    setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(socket_fd_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
}

} // namespace hardware
} // namespace asdevlab
