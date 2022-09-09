all:
	gcc -c rsocket.c -pthread
	ar -rcs librsocket.a rsocket.o
	gcc user1.c -L. -lrsocket -pthread -o user1
	gcc user2.c -L. -lrsocket -pthread -o user2
clean:
	rm rsocket.o librsocket.a user1 user2