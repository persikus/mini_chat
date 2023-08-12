#pragma once

#include <netinet/tcp.h>

namespace peter::shared {

    enum class chat_command : int {
        // First message of a client. Also server's way to confirm a client's user name.
        login = 0,
        // Sent by the server to tell clients about a user that left.
        // Also used a counterpart of the user name confirmation. When a client receives this after an
        // attempted login, the client shuts down.
        logout = 1,
        // Used to send normal chat messages from client to server. Also sent by server to all other clients.
        text = 2
    };

    struct my_message {
        chat_command command;
        char message[500];
        char owner[100];
    };

}