#pragma once

#include <netinet/tcp.h>

namespace peter::shared {

    enum class chat_command : int {
        login = 0,
        logout = 1,
        text  = 2
    };

    struct my_message {
        chat_command command;
        char message[500];
        char owner[100];
    };

}