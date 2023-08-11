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

    auto client = Client("Peter", "127.0.0.1", 8900);
    client.run();

    return 0;
}

Client::Client(std::string &&name, std::string &&ip, short port) : name(name) {
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(port);
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    // Put URL into the socket addr
    auto res = inet_pton(AF_INET, "127.0.0.1", &socket_address.sin_addr);
}

int Client::run() {

    auto my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (my_socket < 0) {
        std::cout << "socket create didnt work: " << errno << std::endl;
        return errno;
    }

    if (connect(my_socket, addr_ptr, sizeof socket_address) < 0) {
        std::cout << "connect failed: " << errno << std::endl;
        close(my_socket);
        return errno;
    }

    std::string input;
    while (true) {

        std::cout << "[You] ";
        std::cin >> input;

        if (input == "exit") break;


        auto a_message = peter::shared::my_message{};
        std::strcpy(a_message.message, input.c_str());


        // send message struct
        auto bytes_sent = send(my_socket, &a_message, sizeof(a_message), 0);
        std::cout << "sent " << bytes_sent << " bytes" << std::endl;
    }


    close(my_socket);
    return 0;
}
