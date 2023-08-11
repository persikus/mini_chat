#include "shared.hpp"

namespace peter::shared {

    sockaddr_in make_socket() {
        // inet socket setup
        auto socket_addr = sockaddr_in();
        socket_addr.sin_family = AF_INET;
        socket_addr.sin_port = htons(8900);
        socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        return socket_addr;
    }

}