all: client server

client:
	gcc src/client/client.c src/common/stream.c src/common/seats.c -o bin/client

server:
	gcc src/server/server.c src/common/stream.c src/common/seats.c src/server/concert.c -o bin/server -lpthread