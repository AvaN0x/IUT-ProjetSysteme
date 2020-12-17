all: client server

client:
	gcc src/client/client.c -o src/client/client

server:
	gcc src/server/server.c -o src/server/server -lpthread