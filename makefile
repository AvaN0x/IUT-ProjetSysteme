all: client server

client:
	gcc src/client/client.c src/utils.c -o src/client/client

server:
	gcc src/server/server.c src/utils.c -o src/server/server -lpthread