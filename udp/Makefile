PORT=1337

all:
	gcc -Wall -o client client.c
	gcc -Wall -o server server.c -lpthread

many: all
	./server $(PORT)&
	./client 127.0.0.1 $(PORT)&
