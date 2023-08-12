# mini_chat

A tiny prototypical chat app written in C++.

Restrictions:
- use berkeley sockets
- pthreads

The project can be build via CMake or using the makefuile. Two separate targets are available:

- `server`, which can then be executed like `./servver <listening_port>`
- `client`, which can then be executed like `./servver <server_ip>:<server_port>`

Two successful logins and some chatting.

```mermaid
sequenceDiagram
    participant Client2
    participant Server
    participant Client1
    Client1->>Server: Login (Marc)
    Server->>Client1: Login (Marc)
    Client2->>Server: Login (John)
    Server->>Client1: Login (John)
    Server->>Client2: Login (John)
    Client1->>Server: Text (Marc: Hello)
    Server->>Client2: Text (Marc: Hello)
```
Two successful logins and an unsuccessul one because of an already given taken name.

```mermaid
sequenceDiagram
    participant Client2
    participant Client1
    participant Server
    participant Client3
    Client1->>Server: Login (Marc)
    Server->>Client1: Login (Marc)
    Client2->>Server: Login (John)
    Server->>Client1: Login (John)
    Server->>Client2: Login (John)
    Client3->>Server: Login (Marc)
    Server->>Client3: Logout (Marc)
    Note right of Client3: Client3 terminates.
```
