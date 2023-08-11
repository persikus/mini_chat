#pragma once

#include <string>
#include <netinet/tcp.h>

class Client {
public:

    explicit Client(std::string &&name, std::string &&ip, short port);

    int run();

private:
    std::string name;
    sockaddr_in socket_address{};
    const sockaddr *addr_ptr = reinterpret_cast<const sockaddr *>(&socket_address);
};