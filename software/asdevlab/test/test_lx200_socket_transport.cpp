#include "asdevlab/hardware/mount/lx200_socket_transport.hpp"

#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <string>
#include <unistd.h>

namespace {

struct ServerContext {
    int listen_fd = -1;
    volatile bool running = true;
};

void* runServer(void* arg) {
    auto* ctx = static_cast<ServerContext*>(arg);
    ctx->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->listen_fd < 0) {
        return nullptr;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(4050);

    if (bind(ctx->listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        close(ctx->listen_fd);
        ctx->listen_fd = -1;
        return nullptr;
    }

    if (listen(ctx->listen_fd, 1) != 0) {
        close(ctx->listen_fd);
        ctx->listen_fd = -1;
        return nullptr;
    }

    int client_fd = accept(ctx->listen_fd, nullptr, nullptr);
    if (client_fd < 0) {
        close(ctx->listen_fd);
        ctx->listen_fd = -1;
        return nullptr;
    }

    char buffer[256]{};
    ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n > 0) {
        send(client_fd, "OK#", 3, 0);
    }

    close(client_fd);
    close(ctx->listen_fd);
    ctx->listen_fd = -1;
    return nullptr;
}

} // namespace

int main() {
    ServerContext server_ctx{};
    pthread_t thread{};
    pthread_create(&thread, nullptr, runServer, &server_ctx);

    usleep(500000);

    asdevlab::hardware::Lx200SocketTransport transport("127.0.0.1", 4050, 1);
    assert(transport.connect("127.0.0.1", 4050));
    std::string response;
    assert(transport.send(":GU", response));
    assert(response == "OK");
    transport.disconnect();
    pthread_join(thread, nullptr);

    std::cout << "lx200 socket transport test passed\n";
    return 0;
}
