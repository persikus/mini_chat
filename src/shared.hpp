#pragma once

#include <netinet/tcp.h>

namespace peter::shared {

    enum class chat_command : int {
        login = 0,
        text  = 1
    };

    struct my_message {
        chat_command command;
        char message[500];
        char owner[100];
    };

}