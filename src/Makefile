PORT=53953
CFLAGS = -DPORT=$(PORT) -Wall -g -std=c99 -Werror

friendme_server: friendme_server.o friends.o 
	gcc $(CFLAGS) -o friendme_server friendme_server.o friends.o

friendme_server.o: friendme_server.c friends.h
	gcc $(CFLAGS) -c friendme_server.c

friends.o: friends.c friends.h
	gcc $(CFLAGS) -c friends.c

clean: 
	rm *.o friendme_server friends
