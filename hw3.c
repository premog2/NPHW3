#include <ctype.h>
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

typedef struct user {
	char *name;
} user;

typedef struct channel {
	char *name;
} channel;


int main(int argc, char *argv[]) {
	char buffer[BUFSIZE];
	char cmd[CMDSIZE];
	int pid, n, optval;
	struct sockaddr_in serveraddr, client;
	socklen_t sockaddr_len, client_len;

	/* port number */
	unsigned short port = 21201;

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

		printf("%d: Received Connection...\n", getpid());

		memset(&buffer, 0, sizeof(buffer));
		memset(&cmd, 0, sizeof(cmd));

		n = recv(newsd, buffer, BUFSIZE, 0);
		if(n < 0) {
			perror("recv()");
			exit(EXIT_FAILURE);
		}

		strncpy(cmd, buffer, 4);
		if(!strcmp(cmd, USER)) {
			printf("%d: welcome USER\n", getpid());
			printf("%s\n", &buffer[0]+5);
		}
		else {
			// todo send this message to client and close connection
			printf("Invalid command, please identify yourself with USER.");
			close(newsd);
		}
		// do {
		// 	n = recv(newsd, buffer, BUFSIZE, 0);
		// 	if(n < 0) {
		// 		perror("recv()");
		// 		exit(EXIT_FAILURE);
		// 	} else if(n == 0) {
		// 		break;
		// 	}
		// 	else {
		// 		// buffer[n] = '\0';		
		// 		printf("%d: Received \"%s\"\n", getpid(), buffer);
		// 	}
		// }
		// while(n > 0);
	}				



	return EXIT_SUCCESS;
}