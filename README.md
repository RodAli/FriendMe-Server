# FriendMe-Server
A social media server that communicates through sockets in C.

![Alt text](/images/diagram.jpg "Diagram of friendme_server data structure")

### Setup server
Compile the code using the Makefile by running 
~~~
make
~~~
in the src folder. Then run friendme_server.c using gcc.

### Communicate to server
You can do this using a tool such Netcat that can read/write over the network through a socket. 
The socket number defined in the Makefile is 53953.

### Commands
~~~
list_users
~~~
List all the users stored in the server.

~~~
make_friends <username>
~~~
Become friends with the user identified with given username.

~~~
post <username> <message>
~~~
Post message given on the users wall identified by username given.

~~~
profile <username>
~~~
View a users profile identified by giving their username.

~~~
quit
~~~
Disconnect from server by closing socket connection.
