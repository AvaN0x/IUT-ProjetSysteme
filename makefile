all: client server

client:
	gcc src/client/client.c src/utils.c -o bin/client

server:
	gcc src/server/server.c src/utils.c -o bin/server -lpthread