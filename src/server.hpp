#pragma once

#include "shared.hpp"
#include <vector>
#include <unordered_set>
#include <unordered_map>

class Server {
public:
    explicit Server(short port);

    int run();

private:


    void broadcast(peter::shared::my_message& message, int except);
    void send_to(peter::shared::my_message& message, int socket);

    sockaddr_in socket_address{};
    const sockaddr *addr_ptr = reinterpret_cast<const sockaddr *>(&socket_address);
    std::unordered_set<int> client_sockets;
    std::unordered_map<int, std::string> socket_owners;

    // Thread method
    static void* handle_client(void* socket);
    void * handle_client(int socket);
    void remove_client(int socket);

};