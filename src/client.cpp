#include "client.hpp"
#include "shared.hpp"

#include <charconv>
#include <sys/wait.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/tcp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <random>
#include <csignal>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Not exactly 2 arguments" << std::endl;
        std::cout << "./client <username> <ip>:<port>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string_view username = argv[1];
    std::string_view input(argv[2]);
    const auto colon_pos = input.find(':');
    if (colon_pos == -1) {
        std::cerr << "No colon ':' found" << std::endl;
        std::cout << "./client <ip>:<port>" << std::endl;
        return EXIT_FAILURE;
    }
    if (colon_pos == 0 || colon_pos == input.length() - 1) {
        std::cerr << "IP or port are empty" << std::endl;
        std::cout << "./client <ip>:<port>" << std::endl;
        return EXIT_FAILURE;
    }

    const auto ip_str = input.substr(0, colon_pos);
    const auto port_str = input.substr(colon_pos + 1, input.length() - colon_pos);

    int port;
    // Not sure why I bothered using string views, but here I am...
    const auto conv_result = std::from_chars(port_str.begin(), port_str.end(), port);
    if (conv_result.ec == std::errc::invalid_argument
        || conv_result.ec == std::errc::result_out_of_range
        || conv_result.ptr != port_str.end()) {

        std::cerr << port_str << " is no valid port" << std::endl;
    }

    std::random_device rd;
    std::uniform_int_distribution<short> num_range{1, 5};
    auto random_number = num_range(rd);

    auto client = Client(username, ip_str, port);
    client.run();

    return 0;
}

Client::Client(std::string_view name, std::string_view ip, short port) : name(name) {
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(port);
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    // Put URL into the socket addr
    auto res = inet_pton(AF_INET, "127.0.0.1", &socket_address.sin_addr);
    if (res < 0) {
        std::cerr << "url doesnt look quite right: " << errno << std::endl;
        std::raise(SIGABRT);
    }
}

struct thread_args {
    Client *client;
    int socket;
};


int Client::run() {

    auto my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (my_socket < 0) {
        std::cerr << "socket create didnt work: " << errno << std::endl;
        return errno;
    }

    if (connect(my_socket, addr_ptr, sizeof socket_address) < 0) {
        std::cerr << "connect failed: " << errno << std::endl;
        close(my_socket);
        return errno;
    }

    auto login_message = peter::shared::my_message{peter::shared::chat_command::login};
    std::strncpy(login_message.owner, name.c_str(), name.size());

    // send message struct
    auto bytes_sent = send(my_socket, &login_message, sizeof(login_message), 0);
    if (bytes_sent < 0) {
        std::cerr << "Login didnt work, abort." << std::endl;
        close(my_socket);
        return errno;
    }

    pthread_t server_thread;
    auto *thread_input = new thread_args{this, my_socket};
    pthread_create(&server_thread, nullptr, handle_server, thread_input);


    std::string input;
    while (true) {

        std::getline(std::cin, input);

        if (input == "exit") {
            exit = true;
            break;
        }

        auto a_message = peter::shared::my_message{peter::shared::chat_command::text};
        std::strncpy(a_message.message, input.c_str(), name.size());

        // send message struct
        bytes_sent = send(my_socket, &a_message, sizeof(a_message), 0);
        if (bytes_sent < 0) {
            std::cerr << "Message didnt work, abort." << std::endl;
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
            if (!exit) {
                std::cerr << "recv on " << socket << " didnt work: " << errno << std::endl;
            }
            std::cout << "Closing this chat. " << std::endl;
            close(socket);
            // Lazy way to deal with server disconnect and end the getline call in main thread
            std::terminate();
        } else if (read_bytes == 0) {
            // disconnect
            std::cout << "server disconnected. Closing this chat. " << std::endl;
            close(socket);
            // Lazy way to deal with server disconnect and end the getline call in main thread
            std::terminate();
        }
        switch (result.command) {
            case peter::shared::chat_command::text: {
                std::cout << result.owner << ": " << result.message << std::endl;
                break;
            }
            case peter::shared::chat_command::login: {
                if (result.owner == name) {
                    std::cout << "Welcome " + name << ". Enter 'exit' to leave the chat." << std::endl;
                } else {
                    std::cout << "(" << result.owner << " joined.)" << std::endl;
                }
                break;
            }
            case peter::shared::chat_command::logout: {
                if (result.owner == name) {
                    std::cerr << "User name \"" << name << "\" already in use. Try another one" << std::endl;
                    std::terminate();
                } else {
                    std::cout << "(" << result.owner << " left.)" << std::endl;
                }
                break;
            }
        }
    }
}

void Client::server_gone() {

}