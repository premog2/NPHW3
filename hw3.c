#include <ctype.h>
#include <stdbool.h>
#include <err.h>
#include <stdio.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>

#define BUFSIZE 1024
#define CMDSIZE 16

char *USER = "USER";
char *LIST = "LIST";
char *JOIN = "JOIN";
char *PART = "PART";
char *OPERATOR = "OPERATOR";
char *KICK = "KICK";
char *PRIVMSG = "PRIVMSG";
char *QUIT = "QUIT";

typedef struct user {
	char *name;
} user;

typedef struct channel {
	char *name;
} channel;

int get_first_word_of_buffer(const char buffer[], int size, char **first_word);

bool is_cmd_USER(char *cmd) {
	return !strcmp(cmd, USER);
}

bool is_cmd_QUIT(char *cmd) {
	return !strcmp(cmd, QUIT);
}

bool is_cmd_ANY(char *cmd) {
	return  strcmp(cmd, USER) == 0 ||
			strcmp(cmd, LIST) == 0 ||
			strcmp(cmd, JOIN) == 0 ||
			strcmp(cmd, PART) == 0 ||
			strcmp(cmd, OPERATOR) == 0 ||
			strcmp(cmd, KICK) == 0 ||
			strcmp(cmd, PRIVMSG) == 0 ||
			strcmp(cmd, QUIT) == 0;
}

int main(int argc, char *argv[]) {
	char buffer[BUFSIZE];
	char *cmd;
	int pid, n, optval;
	struct sockaddr_in serveraddr, client;
	socklen_t sockaddr_len, client_len;

	/* port number */
	// unsigned short port = 21201;

	/* socket */
	int sd = socket(AF_INET, SOCK_STREAM, 0);

	if (sd < 0) {
		perror("socket()");
		exit(EXIT_FAILURE);
	}

	optval = 1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, 
				(const void *)&optval , sizeof(int));

	sockaddr_len = sizeof(serveraddr);
	memset(&serveraddr, 0, sockaddr_len);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(0);
	serveraddr.sin_family = AF_INET;

	client_len = sizeof(client);

	// bind socket
	if((bind(sd, (struct sockaddr*) &serveraddr, sockaddr_len)) < 0) {
		perror("bind()");
		return EXIT_FAILURE;
	}

	getsockname(sd, (struct sockaddr*) &serveraddr, &sockaddr_len);
	printf("Port: %d\n", ntohs(serveraddr.sin_port));
	listen(sd, 5);

	while(1) {

		/* blocking call */ 
		int newsd = accept(sd, (struct sockaddr *)&client, &client_len);
		if(newsd < 0) {
			perror("accept");
			return EXIT_FAILURE;
		}

		// printf("%d: Received Connection...\n", getpid());
		pid = fork();
		if(pid < 0) {
			perror("fork()");
			exit(EXIT_FAILURE);
		}
		else if(pid == 0) { // child

			user u;

			do {
				memset(&buffer, 0, sizeof(buffer));

				n = recv(newsd, buffer, BUFSIZE, 0);
				if(n < 0) {
					perror("recv()");
					exit(EXIT_FAILURE);
				} else if(n == 0) {
					printf("received nothing\n");
					continue;
				}
				else {
					int cmd_len = get_first_word_of_buffer(buffer, n, &cmd);
					printf("Received command %s\n", cmd);

					if(is_cmd_USER(cmd)) {
						printf("Welcome, %s\n", &buffer[0]+cmd_len+1);
						memcpy(u.name, &buffer[0]+cmd_len+1, n-cmd_len);
						printf("%s", u.name);
					}
					else if(is_cmd_QUIT(cmd)) {
						close(newsd);
						break;
					}
					else if(is_cmd_ANY(cmd)) {
						// todo send this message to client and close connection
						printf("Invalid command, please identify yourself with USER.\n");
						close(newsd);
					}
					else {
						printf("Invalid command.\n");
					}
					free(cmd);
				}
			}
			while(n > 0);
		}
		else {
			close(newsd);
			continue;
		}
	}				



	return EXIT_SUCCESS;
}

int get_first_word_of_buffer(const char buffer[], int size, char **first_word) {
	int i;
	*first_word = (char *)calloc(size, sizeof(char));
	for(i = 0; i < size; i++) {
		if(buffer[i] == ' ' || buffer[i] == '\t' || buffer[i] == '\n')
			break;
	}
	memcpy(*first_word, buffer, i);
	return i;
}