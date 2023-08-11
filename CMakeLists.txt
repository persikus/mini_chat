cmake_minimum_required(VERSION 3.26)
project(mini_chat)

set(CMAKE_CXX_STANDARD 17)

add_executable(server src/server.cpp src/server.hpp
        src/shared.cpp src/shared.hpp)
add_executable(client src/client.cpp src/client.hpp
        src/shared.cpp src/shared.hpp)
