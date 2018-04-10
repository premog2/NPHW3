#include <stdio.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <assert.h>

#define BUFSIZE 1024

struct channel {
	char *name;

};


int main(int argc, char *argv[]) {
	char buffer[BUFSIZE];
	int pid, n, optval;
	struct sockaddr_in serveraddr;
	socklen_t sockaddr_len;

	/* port number */
	unsigned short port = 10101;

	/* socket */
	int sd = socket(AF_INET, SOCK_STREAM, 0);

	if (sd < 0) {
		perror("socket() failed");
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

	// bind socket
	if((bind(sd, (struct sockaddr*) &serveraddr, sockaddr_len)) < 0) {
		perror("bind()");
		return EXIT_FAILURE;
	}

	// getsockname(sd, (struct sockaddr*) &serveraddr, &sockaddr_len);
	printf("Port: %d\n", ntohs(serveraddr.sin_port));
	
	while(1) {
	intr_recv:
		printf("%d: Awaiting new connection...\n", getpid()); 

		/* blocking call */ 
		n = recvfrom(sd, buffer, BUFSIZE, 0, (struct sockaddr*) &serveraddr, &sockaddr_len);
		if(n < 0) {
			if(errno == EINTR) goto intr_recv;
			perror("recvfrom()");
			return EXIT_FAILURE;
		}

		printf("%d: Received Connection...\n", getpid());
	}				



	return EXIT_SUCCESS;
}