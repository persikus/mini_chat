#include "server.hpp"
#include "shared.hpp"


#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <iostream>
#include <unistd.h>

int main() {
    // inet socket setup
    auto socket_addr = sockaddr_in();
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(8900);
    socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);


    const auto *addr_ptr = reinterpret_cast<const sockaddr *>(&socket_addr);


    auto my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (my_socket < 0) {
        std::cout << "socket create didnt work: " << errno << std::endl;
        return errno;
    }

    // Bind port
    if (bind(my_socket, addr_ptr, sizeof socket_addr) < 0) {
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


    auto result = peter::shared::my_message{};
    // read_socket is now a socket for an actual connection. Read some stuff
    auto read_bytes = recv(read_socket, &result, sizeof(result), 0);
    std::cout << "read " << read_bytes << " bytes" << std::endl;


    if (read_bytes < 0) {
        std::cout << "recv didnt work: " << errno << std::endl;
        return errno;
    }

    std::cout << result.message << std::endl;


    if (shutdown(read_socket, SHUT_RDWR) == -1) {
        std::cout << "shutdown didnt work: " << errno << std::endl;
        close(read_socket);
        close(my_socket);
        return errno;
    }
    close(read_socket);
    close(my_socket);

    return 0;
}

