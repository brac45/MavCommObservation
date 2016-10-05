#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* This assumes you have the mavlink headers on your include path
	 or in the same folder as this source file */
#include <mavlink.h>

/* Constants */
#define BUFFER_LENGTH 2041 
#define TCP 99
#define UDP 98
#define UNDEFINED 999

/* Echo messages */
void echoMessages(int sock);
/* Parse args 
 * @args int		argc
 *       char** argv
 *       int*	  protocol
 *       int*		port_num
 * @return 1 if successful, 0 if not */
int parseArgs(int, char**, int*, int*);
/* Show usage */
void usage();

/* Main function */
int main(int argc, char* argv[]) {
	// Variables
	struct sockaddr_in locaddr;
	int	fd;
	int port_num = 0;
	int protocol = UNDEFINED;

	/* Parse arguments */
	if (!parseArgs(argc, argv, &protocol, &port_num)) {
		usage();
		exit(1);
	}

	/* Create a socket for datagrams 
	 * IP protocol family: AF_INET
	 * UDP protocol: SOCK_DGRAM */
	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("Error creating socket(main)");
		exit(1);
	}

	/* Fill out addresses
	 * */
	memset((char*)&locaddr, 0, sizeof(locaddr));
	locaddr.sin_family = AF_INET;
	locaddr.sin_port = htons(port_num);
	locaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Bind socket to port */
	if (bind(fd, (struct sockaddr*)&locaddr, sizeof(locaddr)) == -1) {
		perror("Error while binding(main)");
		exit(1);
	}

	/* Start echoing routine */
	echoMessages(fd);
}

void echoMessages(int sock) {
	int recv_len;
	char buf[BUFFER_LENGTH];
	struct sockaddr_in remote;
	unsigned int s_size = sizeof(remote);

	while(1) {
		fflush(stdout);
		memset(buf, '\0', BUFFER_LENGTH);

		printf("Waiting for datagrams..\n");
		/* Receive data, this is a blocking call */
		if ((recv_len = recvfrom(sock, buf, BUFFER_LENGTH, 0, (struct sockaddr *)&remote, &s_size)) == -1) {
			perror("recvfrom failed from echoMessages(int)");
		}

		/* Print datagram details */
		printf("Received packet from %s:%d\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
		printf("Data: %s\n", buf);

		/* Echo back */
		if (sendto(sock, buf, recv_len, 0, (struct sockaddr *)&remote, s_size) == -1) {
			perror("sendto() failed");
		}
	}
}

int parseArgs(int argc, char** argv, int* protocol, int* port_num) {
	/* TCP or UDP */
	if (argc == 4) {
		/* TCP or UDP */
		if (strcmp("-t", argv[1]) == 0) {
			*protocol = TCP;
		} else if (strcmp("-u", argv[1]) == 0) {
			*protocol = UDP;
		} else {
			return 0;
		}

		/* Port number*/
		if (strcmp("--port", argv[2]) == 0) {
			*port_num = atoi(argv[3]);
			return 1;
		} else {
			printf("[DEBUG] incorrect argv[2]\n");
			return 0;
		}
	} else {
		printf("[DEBUG] incorrect number of args\n");
		/* Incorrect number of args */
		return 0;
	}
}

void usage() {
	fprintf(stdout, "usage..\n");
	fprintf(stdout, "./$(PROGRAM) [-t | -u] [--port PORT]\n");
	fprintf(stdout, "		-t | -u : tcp or udp protocol (only one may be used)\n");
	fprintf(stdout, "		--port : port number to be listened to\n");
}
