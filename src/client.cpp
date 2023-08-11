#include "client.hpp"
#include "shared.hpp"


#include <sys/wait.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/tcp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>


int main() {
    using namespace std::chrono_literals;

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

    // Put URL into the socket addr
    auto res = inet_pton(AF_INET, "127.0.0.1", &socket_addr.sin_addr);

    if (res <= 0) {
        std::cout << "address not fine" << std::endl;
    }

    if (connect(my_socket, addr_ptr, sizeof socket_addr) < 0) {
        std::cout << "connect failed: " << errno << std::endl;
        close(my_socket);
        return errno;
    }

    auto a_message = peter::shared::my_message{"Hello Server"};

    // send message struct
    auto bytes_sent = send(my_socket, &a_message, sizeof(a_message), 0);
    std::cout << "sent " << bytes_sent << " bytes" << std::endl;

    close(my_socket);

    return 0;
}
