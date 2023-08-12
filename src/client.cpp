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
#include <random>

int main() {
    std::random_device rd;
    std::uniform_int_distribution<short> num_range {1, 9};
    auto random_number = num_range(rd);

    auto client = Client("Peter_" + std::to_string(random_number), "127.0.0.1", 8900);
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

struct thread_args {
    Client *client;
    int socket;
};


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

    auto login_message = peter::shared::my_message{peter::shared::chat_command::login};
    std::strcpy(login_message.message, name.c_str());

    // send message struct
    auto bytes_sent = send(my_socket, &login_message, sizeof(login_message), 0);
    if (bytes_sent < 0) {
        std::cout << "Login didnt work, abort." << std::endl;
        close(my_socket);
        return errno;
    }

    pthread_t server_thread;
    auto* thread_input = new thread_args{this, my_socket};
    std::cout << "give " << my_socket << std::endl;
    pthread_create(&server_thread, nullptr, handle_server, thread_input);


    std::string input;
    while (true) {

        std::getline(std::cin, input);

        if (input == "exit") break;

        auto a_message = peter::shared::my_message{peter::shared::chat_command::text};
        std::strcpy(a_message.message, input.c_str());


        // send message struct
        bytes_sent = send(my_socket, &a_message, sizeof(a_message), 0);
        if (bytes_sent < 0) {
            std::cout << "Message didnt work, abort." << std::endl;
            close(my_socket);
            return errno;
        }
    }


    close(my_socket);
    return 0;
}


void *Client::handle_server(void *args) {
    // "Take" ownership. Helps some analyzers. Stupid raw pointers..
    const auto args_ptr = std::unique_ptr<thread_args>((thread_args *) args);
    auto &client = *args_ptr->client;
    auto socket = args_ptr->socket;
    // Move this method to the client. Makes things easier.
    return client.handle_server(socket);
}

void *Client::handle_server(int socket) {
    auto result = peter::shared::my_message{};
    while (true) {
        // read_socket is now a socket for an actual connection. Read some stuff
        auto read_bytes = recv(socket, &result, sizeof(result), 0);
        if (read_bytes < 0) {
            std::cout << "recv on " << socket << " didnt work: " << errno << std::endl;
            server_gone();
            pthread_exit(nullptr);
        } else if (read_bytes == 0) {
            // disconnect, I think
            if (shutdown(socket, SHUT_RDWR) == -1) {
                std::cout << "shutdown didnt work: " << errno << std::endl;
                server_gone();
            }
            close(socket);
            pthread_exit(nullptr);
        }
        switch (result.command) {
            case peter::shared::chat_command::text: {
                std::cout << result.owner << ": " << result.message << std::endl;
                break;
            }
            case peter::shared::chat_command::login: {
                std::cout << result.owner << " joined." << std::endl;
            }
        }
    }
}

void Client::server_gone() {

}