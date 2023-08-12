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

struct thread_args {
    Server *server;
    int socket;
};

const int enable = 1;

int Server::run() {

    auto my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (setsockopt(my_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl;
    }
    if (setsockopt(my_socket, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        std::cerr << "setsockopt(SO_REUSEPORT) failed" << std::endl;
    }

    if (my_socket < 0) {
        std::cerr << "socket create didnt work: " << errno << std::endl;
        return errno;
    }

    // Bind port
    if (bind(my_socket, addr_ptr, sizeof socket_address) < 0) {
        std::cerr << "bind didnt work: " << errno << std::endl;
        close(my_socket);
        return errno;
    }

    // Allow connections to queue up
    if (listen(my_socket, 2) == -1) {
        std::cerr << "listen didnt work: " << errno << std::endl;
        perror("listen failed");
        close(my_socket);
        return errno;
    }

    // Now a lot of connections may be queued

    struct sockaddr_in their_addr{}; // connector’s address
    socklen_t sin_size; // size of sockaddr

    std::vector<pthread_t> client_threads;

    while (true) {
        auto read_socket = -1;
        std::cout << "Waiting for connection.." << std::endl;
        if ((read_socket = accept(my_socket, (struct sockaddr *) &their_addr, &sin_size)) < 0) {
            std::cout << "accept deque didnt work: " << errno << std::endl;
            break;
        }
        std::cout << "Someone connected on socket " << read_socket << std::endl;

        client_threads.emplace_back();
        client_sockets.insert(read_socket);

        auto input = new thread_args{this, read_socket};
        std::cout << "give " << read_socket << std::endl;
        pthread_create(&client_threads.back(), nullptr, handle_client, input);
    }

    close(my_socket);
    return 0;
}

void *Server::handle_client(void *args) {
    // "Take" ownership. Helps some analyzers. Stupid raw pointers..
    const auto args_ptr = std::unique_ptr<thread_args>((thread_args *) args);
    auto &server = *args_ptr->server;
    auto socket = args_ptr->socket;
    // Move this method to the client. Makes things easier.
    return server.handle_client(socket);
}

void *Server::handle_client(int socket) {
    auto result = peter::shared::my_message{};
    while (true) {
        // read_socket is now a socket for an actual connection. Read some stuff
        auto read_bytes = recv(socket, &result, sizeof(result), 0);
        if (read_bytes < 0) {
            std::cout << "recv on " << socket << " didnt work: " << errno << std::endl;
            remove_client(socket);
            return nullptr;
        } else if (read_bytes == 0) {
            // disconnect
            std::cout << "(" << socket_owners[socket] << " left.)" << std::endl;
            result.command = peter::shared::chat_command::logout;
            broadcast(result, socket);
            remove_client(socket);
            return nullptr;
        }
        switch (result.command) {
            case peter::shared::chat_command::login: {
                std::cout << "(" << result.message << " joined.)" << std::endl;
                client_sockets.insert(socket);
                socket_owners[socket] = result.message;
                broadcast(result, socket);
                break;
            }
            case peter::shared::chat_command::text: {
                std::cout << "[" << socket_owners[socket] << "] " << result.message << std::endl;
                broadcast(result, socket);
                break;
            }
            default:
                std::cerr << "unexpected client command " << static_cast<int>(result.command) << std::endl;
                break;
        }
    }
}

void Server::broadcast(peter::shared::my_message &message, int except) {
    std::strncpy(message.owner, socket_owners[except].c_str(), socket_owners[except].size());
    for (const auto socket: client_sockets) {
        if (socket == except) continue;
        std::cout << "send this message to " << socket << std::endl;
        auto bytes_sent = send(socket, &message, sizeof(message), 0);
        if (bytes_sent < 0) {
            std::cerr << "broadcast didnt work" << std::endl;
        }
    }
}

Server::Server(short port) {
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(8900);
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
}

void Server::remove_client(int socket) {
    close(socket);
    client_sockets.erase(socket);
    socket_owners.erase(socket);
}
