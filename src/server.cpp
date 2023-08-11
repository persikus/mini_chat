#include "server.hpp"


#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <iostream>
#include <unistd.h>

int main() {

    auto server = Server(8900);
    return server.run();
}

int Server::run() {

    auto my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (my_socket < 0) {
        std::cout << "socket create didnt work: " << errno << std::endl;
        return errno;
    }

    // Bind port
    if (bind(my_socket, addr_ptr, sizeof socket_address) < 0) {
        std::cout << "bind didnt work: " << errno << std::endl;
        close(my_socket);
        return errno;
    }

    // Allow connections to queue up
    if (listen(my_socket, 2) == -1) {
        std::cout << "listen didnt work: " << errno << std::endl;
        perror("listen failed");
        close(my_socket);
        return errno;
    }

    // Now a lot of connections may be queued

    struct sockaddr_in their_addr{}; // connector’s address
    socklen_t sin_size; // size of sockaddr


    auto read_socket = -1;
    std::cout << "Waiting for connection.." << std::endl;
    if ((read_socket = accept(my_socket, (struct sockaddr *) &their_addr, &sin_size)) < 0) {
        std::cout << "accept deque didnt work: " << errno << std::endl;
    }
    std::cout << "Got one" << std::endl;

    handle_client(read_socket);

    close(my_socket);
}

void Server::handle_client(int socket) {
    auto result = peter::shared::my_message{};
    while (true) {
        // read_socket is now a socket for an actual connection. Read some stuff
        auto read_bytes = recv(socket, &result, sizeof(result), 0);
        if (read_bytes < 0) {
            std::cout << "recv didnt work: " << errno << std::endl;
            return;
        }
        else if (read_bytes == 0) {
            // disconnect, I think
            if (shutdown(socket, SHUT_RDWR) == -1) {
                std::cout << "shutdown didnt work: " << errno << std::endl;
            }
            close(socket);
            return;
        }
        std::cout << result.message << std::endl;
    }
}

void Server::broadcast(const peter::shared::my_message message) {

}

Server::Server(short port) {
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(8900);
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
}
