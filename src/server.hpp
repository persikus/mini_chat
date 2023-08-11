#pragma once

#include "shared.hpp"
#include <vector>

class Server {
public:
    explicit Server(short port);

    int run();

private:

    static void* handle_client(void* socket);

    void broadcast(peter::shared::my_message message);

    sockaddr_in socket_address{};
    const sockaddr *addr_ptr = reinterpret_cast<const sockaddr *>(&socket_address);

};