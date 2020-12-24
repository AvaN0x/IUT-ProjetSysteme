all: client server

client:
	gcc src/client/client.c src/common/stream.c -o bin/client

server:
	gcc src/server/server.c src/common/stream.c -o bin/server -lpthread