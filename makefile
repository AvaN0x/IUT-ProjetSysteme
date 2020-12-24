all: client server

client:
	gcc src/client/client.c src/common/*.c -o bin/client

server:
	gcc src/server/server.c src/common/*.c src/server/concert.c -o bin/server -lpthread