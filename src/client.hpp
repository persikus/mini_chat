#pragma once

#include <string>
#include <netinet/tcp.h>

class Client {
public:

    explicit Client(std::string_view name, std::string_view ip, short port);

    int run();

private:
    std::string name;
    sockaddr_in socket_address{};
    const sockaddr *addr_ptr = reinterpret_cast<const sockaddr *>(&socket_address);

    bool exit;

    static void* handle_server(void* socket);
    void * handle_server(int socket);
    void server_gone();
};