#include "friends.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/*
 * Create a new user with the given name.  Insert it at the tail of the list 
 * of users whose head is pointed to by *user_ptr_add.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if a user by this name already exists in this list.
 *   - 2 if the given name cannot fit in the 'name' array
 *       (don't forget about the null terminator).
 */
int create_user(const char *name, User **user_ptr_add) {
    if (strlen(name) >= MAX_NAME) {
        return 2;
    }

    User *new_user = malloc(sizeof(User));
    if (new_user == NULL) {
        perror("malloc");
        exit(1);
    }
    strncpy(new_user->name, name, MAX_NAME); // name has max length MAX_NAME - 1

    for (int i = 0; i < MAX_NAME; i++) {
        new_user->profile_pic[i] = '\0';
    }

    new_user->first_post = NULL;
    new_user->next = NULL;
    for (int i = 0; i < MAX_FRIENDS; i++) {
        new_user->friends[i] = NULL;
    }

    // Add user to list
    User *prev = NULL;
    User *curr = *user_ptr_add;
    while (curr != NULL && strcmp(curr->name, name) != 0) {
        prev = curr;
        curr = curr->next;
    }

    if (*user_ptr_add == NULL) {
        *user_ptr_add = new_user;
        return 0;
    } else if (curr != NULL) {
        free(new_user);
        return 1;
    } else {
        prev->next = new_user;
        return 0;
    }
}


/* 
 * Return a pointer to the user with this name in
 * the list starting with head. Return NULL if no such user exists.
 */
User *find_user(const char *name, const User *head) {
/*    const User *curr = head;
    while (curr != NULL && strcmp(name, curr->name) != 0) {
        curr = curr->next;
    }

    return (User *)curr;
*/
    while (head != NULL && strcmp(name, head->name) != 0) {
        head = head->next;
    }

    return (User *)head;
}


/*
 * Print the usernames of all users in the list starting at curr.
 * Names should be printed to standard output, one per line.
 */
char *list_users(const User *curr) {

    int list_size = 0;
    User *current = (User*)curr;

    //Increment through each user to determine the size of the list
    while (curr != NULL) {

        list_size ++;
        curr = curr->next;
    }
	
	
    if (list_size > 0){
        char *user_list = malloc((sizeof(char) * MAX_FRIENDS * list_size) + (list_size * 2));
        strcpy(user_list, current->name);
        strcat(user_list, "\r\n");
        current = current->next;

        while (current != NULL) {
            strcat(user_list, current->name);
            strcat(user_list, "\r\n");
            current = current->next;
        }
		
		return user_list;
    }

	// return the empty string
	char *empty = malloc(sizeof(char));
    return empty;

}



/* 
 * Make two users friends with each other.  This is symmetric - a pointer to 
 * each user must be stored in the 'friends' array of the other.
 *
 * New friends must be added in the first empty spot in the 'friends' array.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if the two users are already friends.
 *   - 2 if the users are not already friends, but at least one already has
 *     MAX_FRIENDS friends.
 *   - 3 if the same user is passed in twice.
 *   - 4 if at least one user does not exist.
 */
int make_friends(const char *name1, const char *name2, User *head) {
    User *user1 = find_user(name1, head);
    User *user2 = find_user(name2, head);

    if (user1 == NULL || user2 == NULL) {
        return 4;
    } else if (user1 == user2) { // Same user
        return 3;
    }

    int i, j;
    for (i = 0; i < MAX_FRIENDS; i++) {
        if (user1->friends[i] == NULL) { // Empty spot
            break;
        } else if (user1->friends[i] == user2) { // Already friends.
            return 1;
        }
    }

    for (j = 0; j < MAX_FRIENDS; j++) {
        if (user2->friends[j] == NULL) { // Empty spot
            break;
        } 
    }

    if (i == MAX_FRIENDS || j == MAX_FRIENDS) { // Too many friends.
        return 2;
    }

    user1->friends[i] = user2;
    user2->friends[j] = user1;
    return 0;
}


/* 
 * Print a user profile.
 * For an example of the required output format, see the example output
 * linked from the handout.
 * Return:
 *   - 0 on success.
 *   - 1 if the user is NULL.
 */
char *print_user(const User *user) {
    if (user == NULL) {
		char *empty = malloc(sizeof(char));
        return empty;
    }
	
	int post_size = 0;
	const Post *curr = user->first_post;
	while (curr != NULL) {
		post_size += strlen(curr->author) + strlen(asctime(localtime(curr->date))) 
										  + strlen(curr->contents) + 29;
		curr = curr->next;
    }
	
	char *user_content = malloc((sizeof(char) * MAX_FRIENDS * MAX_NAME) + (MAX_FRIENDS * 2)
									+ (sizeof(char) * post_size) + 151);
									
	// Copy the name
	strcpy(user_content, "Name: ");
	strcat(user_content, user->name);
	strcat(user_content, "\r\n\r\n");
	strcat(user_content, "------------------------------------------\r\n");

    // Copy friend list.
	strcat(user_content, "Friends:\r\n");
    for (int i = 0; i < MAX_FRIENDS && user->friends[i] != NULL; i++) {
		strcat(user_content, user->friends[i]->name);
		strcat(user_content, "\r\n");
    }
	strcat(user_content, "------------------------------------------\r\n");

    // Copy post list.
	strcat(user_content, "Posts:\r\n");
    const Post *post = user->first_post;
    while (post != NULL) {	
        
		// Copy the author
		strcat(user_content, "From: ");
		strcat(user_content, post->author);
		strcat(user_content, "\r\n");
		
		// Copy the date
		strcat(user_content, "Date: ");
		strcat(user_content, asctime(localtime(post->date)));
		strcat(user_content, "\r\n");
		
		// Copy the message
		strcat(user_content, post->contents);
		strcat(user_content, "\r\n");
		
        post = post->next;
        if (post != NULL) {
			strcat(user_content, "\r\n===\r\n\r\n");
        }
    }
	
	strcat(user_content, "------------------------------------------\r\n");

    return user_content;
}


/*
 * Make a new post from 'author' to the 'target' user,
 * containing the given contents, IF the users are friends.
 *
 * Insert the new post at the *front* of the user's list of posts.
 *
 * Use the 'time' function to store the current time.
 *
 * 'contents' is a pointer to heap-allocated memory - you do not need
 * to allocate more memory to store the contents of the post.
 *
 * Return:
 *   - 0 on success
 *   - 1 if users exist but are not friends
 *   - 2 if either User pointer is NULL
 */
int make_post(const User *author, User *target, char *contents) {
    if (target == NULL || author == NULL) {
        return 2;
    }

    int friends = 0;
    for (int i = 0; i < MAX_FRIENDS && target->friends[i] != NULL; i++) {
        if (strcmp(target->friends[i]->name, author->name) == 0) {
            friends = 1;
            break;
        }
    }

    if (friends == 0) {
        return 1;
    }

    // Create post
    Post *new_post = malloc(sizeof(Post));
    if (new_post == NULL) {
        perror("malloc");
        exit(1);
    }
    strncpy(new_post->author, author->name, MAX_NAME);
    new_post->contents = contents;
    new_post->date = malloc(sizeof(time_t));
    if (new_post->date == NULL) {
        perror("malloc");
        exit(1);
    }
    time(new_post->date);
    new_post->next = target->first_post;
    target->first_post = new_post;

    return 0;
}