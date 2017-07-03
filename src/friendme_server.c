#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include "friends.h"

#ifndef PORT
  #define PORT 53953
#endif
#define BUFFER_SIZE 256
#define INPUT_ARG_MAX_NUM 12
#define DELIM " \n"

// Global Variables:
static int listen_fd;

struct client {
    int fd;
    char name[MAX_NAME];
    struct client *next;
};

// Head of active client list
struct client *active_clients = NULL;
// Head of users stored in program
User *user_list = NULL;


void setup(void) {
	int status;

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("socket");
	exit(1);
	}

	// Make sure we can reuse the port immediately after the
	// server terminates.
	int on = 1;
	status = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
	                  (const char *) &on, sizeof(on));
	if(status == -1) {
	perror("setsockopt -- REUSEADDR");
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);
	memset(&server.sin_zero, 0, sizeof(server.sin_zero));  // Initialize sin_zero to 0

	if (bind(listen_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
	perror("bind"); // probably means port is in use
	exit(1);
	}

	if (listen(listen_fd, 5) == -1) {
	perror("listen");
	exit(1);
	}
}

/*
 * Takes in a file descriptor to print a message to 
 * signify invalid input.
 */
void error(char *msg, int fd){
	write(fd, msg, strlen(msg) + 1);
}


/*
 * Tokenize the string stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 *
 * This tokenize function was used from assignment 3 starter code.
 */ 
int tokenize(char *cmd, char **cmd_argv, int fd) {
    int cmd_argc = 0;
    char *next_token = strtok(cmd, DELIM);    
    while (next_token != NULL) {
        if (cmd_argc >= INPUT_ARG_MAX_NUM - 1) {
            error("Too many arguments!\r\n" , fd);
            cmd_argc = 0;
            break;
        }
        cmd_argv[cmd_argc] = next_token;
        cmd_argc++;
        next_token = strtok(NULL, DELIM);
    }

    return cmd_argc;
}


/* 
 * Read and process commands.
 *
 * Modified version of process args from assignment 3 starter code
 */ 
int process_args(int cmd_argc, char **cmd_argv, User **user_list_ptr, int fd, User *user_ptr) {
    User *user_list = *user_list_ptr;

    if (cmd_argc <= 0) {
        return 0;
    } else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
        return -1;
    } else if (strcmp(cmd_argv[0], "list_users") == 0 && cmd_argc == 1) {
		char *buf;
        buf = list_users(user_list);
		write(fd, buf, strlen(buf) + 1);
		free(buf);
		
    } else if (strcmp(cmd_argv[0], "make_friends") == 0 && cmd_argc == 2) {
        char buf[BUFFER_SIZE];
        switch (make_friends(user_ptr->name, cmd_argv[1], user_list)) {
            case 0:
            	strcpy(buf, "You are now friends with ");
            	strcat(buf, cmd_argv[1]);
            	strcat(buf, "\r\n");
            	write(fd, buf, strlen(buf));
            	break;
            case 1:
                error("You are already friends.\r\n", fd);
                break;
            case 2:
                error("At least one of you entered has the max number of friends\r\n", fd);
                break;
            case 3:
                error("You can't friend yourself\r\n", fd);
                break;
            case 4:
                error("The user you entered does not exist\r\n", fd);
                break;
        }
    } else if (strcmp(cmd_argv[0], "post") == 0 && cmd_argc >= 3) {
        // first determine how long a string we need
        int space_needed = 0;
        for (int i = 2; i < cmd_argc; i++) {
            space_needed += strlen(cmd_argv[i]) + 1;
        }

        // allocate the space
        char *contents = malloc(space_needed);
        if (contents == NULL) {
            perror("malloc");
            exit(1);
        }

        // copy in the bits to make a single string
        strcpy(contents, cmd_argv[2]);
        for (int i = 3; i < cmd_argc; i++) {
            strcat(contents, " ");
            strcat(contents, cmd_argv[i]);
        }

        User *target = find_user(cmd_argv[1], user_list);
        switch (make_post(user_ptr, target, contents)) {
            case 1:
                error("You can only post to your friends\r\n", fd);
                break;
            case 2:
                error("The user you want to post to does not exist\r\n", fd);
                break;
        }
    } else if (strcmp(cmd_argv[0], "profile") == 0 && cmd_argc == 2) {
        User *user = find_user(cmd_argv[1], user_list);
        if (strcmp(print_user(user), "") == 0) {
            error("User not found\r\n", fd);
        } else {
			char *buf = print_user(user);
			write(fd, buf, strlen(buf) + 1);
			free(buf);
		}
    } else {
        error("Incorrect syntax\r\n", fd);
    }
    return 0;
} 


int find_newline(const char *buf, int inbuf) {
	
	for (int i = 0; i < inbuf; i++){
		if ((buf[i] == '\r') && (buf[i + 1] == '\n')){
			return i;
		}
	}
	return -1;
}


void add_client(){
	int fd;

	struct sockaddr_in peer;
	socklen_t socklen;

	// Accept the new client
	if ((fd = accept(listen_fd, (struct sockaddr *)&peer, &socklen)) < 0) {
		perror("accept");
    } 

    char greeting[26] = "What is your user name?\r\n";
    write(fd, greeting, sizeof(greeting));

    // Create the new client and add to the server list
    struct client *new_client = malloc(sizeof(struct client));
    if (!new_client){
    	perror("malloc");
    	exit(1);
    }

    new_client->fd = fd;
    new_client->next = NULL;
    // Start with blank name
    for (int i = 0; i < MAX_NAME; i++){
    	new_client->name[i] = '\0';
    }

    struct client *current = active_clients;

    if (active_clients == NULL){
    	active_clients = new_client;
    }else{
    	while(current->next != NULL){
    		current = current->next;
    	}
    	current->next = new_client;
    }

}

void read_client(struct client *client_ptr){

	int nbytes;
	char buf[BUFFER_SIZE];
	int inbuf; // how many bytes currently in buffer?
	int room; // how much room left in buffer?
	char *after; // pointer to position after the (valid) data in buf
	int where; // location of network newline

	inbuf = 0;          // buffer is empty; has no bytes
	room = sizeof(buf); // room == capacity of the whole buffer
	after = buf;        // start writing at beginning of buf

	while ((nbytes = read(client_ptr->fd, after, room)) > 0) {
		
		inbuf += nbytes;

		where = find_newline(buf, inbuf);

		// When we have full line, null terminators to end string
		// and exit the reading of the buffer
		if (where >= 0) {

			buf[where] = '\0';
			buf[where + 1] = '\0';
			break;				
		}

		room = sizeof(buf) - inbuf;
		after = &buf[inbuf];
	}

	// Check if the user had not given a name yet
	if (client_ptr->name[0] == '\0'){

		// Check if client is already in the program
		User *user_ptr;
		int client_found = 0;
		for (user_ptr = user_list; user_ptr; user_ptr = user_ptr->next) {
		    if (strcmp(user_ptr->name, buf) == 0) {
		    	client_found = 1;
		    }
		}


		if (client_found == 1){
			// Add client to active clients, welcome back new client
			char message[16] = "Welcome back.\r\n";
    		write(client_ptr->fd, message, sizeof(message));
    		strncpy(client_ptr->name, buf, sizeof(client_ptr->name) - 1);
		} else {

			// Check if the name is too long
			if (strlen(buf) > MAX_NAME - 1){
				char err_msg[44] = "Username too long, truncated to 31 chars.\r\n";
				write(client_ptr->fd, err_msg, sizeof(err_msg));
			} else {
				char message[11] = "Welcome.\r\n";
				write(client_ptr->fd, message, sizeof(message));
			}
			// Add client to active clients
			strncpy(client_ptr->name, buf, sizeof(client_ptr->name) - 1);

			// Create the new user and store in system
			create_user(client_ptr->name, &user_list);
		}

		char prompt[36] = "Go ahead and enter user commands>\r\n";
		write(client_ptr->fd, prompt, sizeof(prompt));
		
	} else {
		
		
		char *cmd_argv[INPUT_ARG_MAX_NUM];
		int cmd_argc = tokenize(buf, cmd_argv, client_ptr->fd);

		// Find this user and point this pointer to this user
		User *user_ptr;
		for (user_ptr = user_list; user_ptr; user_ptr = user_ptr->next) {
		    if (strcmp(user_ptr->name, client_ptr->name) == 0) {
		    	break;
		    }
		}
		
		if (cmd_argc > 0 && process_args(cmd_argc,
				 cmd_argv, &user_list, client_ptr->fd, user_ptr) == -1) {

			struct client *curr = active_clients;
			while (curr != NULL){
				struct client* next_one = curr->next;

				//Handles case where the user is at the head of the client list
				if ((curr == client_ptr) && (next_one != NULL)){
					active_clients = next_one;
					struct client **delete_client = &client_ptr;
					close(client_ptr->fd);
        	        free(*delete_client);
        	        break;

        	    //Handles the case where the user is not at the head of the list
				}else if(next_one == client_ptr){
					struct client **delete_client = &client_ptr;
					curr->next = curr->next->next;
					close(client_ptr->fd);
        	        free(*delete_client);
				}else{
					curr = curr->next;
				}
			}	
        }
		
	}
}



int main(){

	// Setup the socket
	setup();

	// Client pointer to loop through online clients
	struct client *client_ptr;
	
	// Make server in infinite loop until the program is killed
	while (1){

		fd_set fdlist;
		int maxfd = listen_fd;
		FD_ZERO(&fdlist);
		FD_SET(listen_fd, &fdlist);

		// Loop through clients find the max file desciptor
		for (client_ptr = active_clients; client_ptr; client_ptr = client_ptr->next) {
		    FD_SET(client_ptr->fd, &fdlist);
		    if (client_ptr->fd > maxfd){
		    	maxfd = client_ptr->fd;
		    }
		}

		if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
		    perror("select");
		}

		// Loop through clients to find client ready for reading
		for (client_ptr = active_clients; client_ptr; client_ptr = client_ptr->next) {
		    if (FD_ISSET(client_ptr->fd, &fdlist)){
		    	break;
		    }
		}

		struct client *ptr;
		for (ptr = active_clients; ptr; ptr = ptr->next) {
		    printf("%s ->", ptr->name);
		}
		printf("\n");	

		// Read from client if client is ready
		if (client_ptr){
			read_client(client_ptr);
		}

		if (FD_ISSET(listen_fd, &fdlist)){
			add_client();
		}

		

	}
	return 0;
}