all: client server

client: client.o
	g++ -o client client.o shared.o

client.o: src/client.cpp src/shared.cpp
	g++ -c -std=c++17 -pthread src/client.cpp src/shared.cpp

server: server.o
	g++ -o server server.o shared.o

server.o: src/server.cpp src/shared.cpp
	g++ -c -std=c++17 -pthread src/server.cpp src/shared.cpp
