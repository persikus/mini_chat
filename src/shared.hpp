#pragma once

#include <netinet/tcp.h>

namespace peter::shared {

    struct my_message {
        char message[1000];
    };

}