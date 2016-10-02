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
#define PORT_NUM			8888 // Port number

/* Echo messages */
void echoMessages(int sock);

/* Main function */
int main(int argc, char* argv[]) {
	// Variables
	struct sockaddr_in locaddr;
	int	fd;
	int i;
	int len;

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
	locaddr.sin_port = htons(PORT_NUM);
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
	int s_size = sizeof(remote);

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
