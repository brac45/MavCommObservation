#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>

/* This assumes you have the mavlink headers on your include path
	 or in the same folder as this source file */
#include <mavlink.h>

/* Constants */
#define BUFFER_LENGTH 2041 
#define TCP 99
#define UDP 98
#define UNDEFINED 999

/* Echo messages */
void serverRoutine(int sock);
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
	serverRoutine(fd);
}

void serverRoutine(int sock) {
	int i = 0;
	mavlink_message_t mavmsg, newmsg;
	mavlink_status_t status;
	struct timeval tv;
	int recv_len, retlen;
	uint8_t buf[BUFFER_LENGTH];
	double timestamp;
	struct sockaddr_in remote;
	unsigned int s_size = sizeof(remote);

	while(1) {
		memset((char*)buf, 0, BUFFER_LENGTH);
		memset((char*)&mavmsg, 0, sizeof(mavmsg));
		memset((char*)&newmsg, 0, sizeof(newmsg));
		memset((char*)&status, 0, sizeof(status));

		printf("Waiting for datagrams..\n");
		/* Receive data, this is a blocking call */
		if ((recv_len = recvfrom(sock, buf, BUFFER_LENGTH, 0, (struct sockaddr *)&remote, &s_size)) < 0) {
			fprintf(stderr, "recvfrom failed from serverRoutine(int)");
		} else {
			/* Parse packet */
			for (i=0; i<recv_len; i++) {
				if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &mavmsg, &status)) {
					if (mavmsg.msgid == MAVLINK_MSG_ID_TEST_FRAME) {
						/* Indicate packet is received */
						fprintf(stdout, "%d bytes received(%s:%d)\n",
								(int)recv_len, inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
						fprintf(stdout, "[DEBUG] seq_num: %u, timestamp_sender: %lf, timestamp_echo: %lf\n",
								mavlink_msg_test_frame_get_sequence(&mavmsg),
								mavlink_msg_test_frame_get_timestamp_sender(&mavmsg),
								mavlink_msg_test_frame_get_timestamp_echo(&mavmsg));

						/* Get current timestamp */
						gettimeofday(&tv, NULL);
						timestamp = ((double)(tv.tv_sec) * 1000) 
							+ ((double)(tv.tv_usec) / 1000);

						/* Reset buffer */
						memset((char*)buf, 0, BUFFER_LENGTH);

						/* Repack message with timestamp */
						mavlink_msg_test_frame_pack(1, 200, &newmsg, 
								mavlink_msg_test_frame_get_sequence(&mavmsg),
								mavlink_msg_test_frame_get_timestamp_sender(&mavmsg),
								timestamp);
						retlen = mavlink_msg_to_send_buffer(buf, &newmsg);

						/* Debug string */
						fprintf(stdout, "[DEBUG] seq_num: %u, timestamp_sender: %lf, timestamp_echo: %lf\n",
								mavlink_msg_test_frame_get_sequence(&newmsg),
								mavlink_msg_test_frame_get_timestamp_sender(&newmsg),
								mavlink_msg_test_frame_get_timestamp_echo(&newmsg));

						/* Echo back */
						if (sendto(sock, buf, retlen, 0, (struct sockaddr *)&remote, s_size) == -1) {
							perror("sendto() failed");
						}
					}
				}
			}
		}
	}
}

int parseArgs(int argc, char** argv, int* protocol, int* port_num) {
	/* TCP or UDP */
	if (argc == 3) {
		/* db path */
		//strcpy(db_path, argv[1]);

		/* session id */
		//strcpy(session_id, argv[2]);

		/* TCP or UDP */
		if (strcmp("-t", argv[1]) == 0) {
			*protocol = TCP;
		} else if (strcmp("-u", argv[1]) == 0) {
			*protocol = UDP;
		} else {
			return 0;
		}

		/* Port number*/
		*port_num = atoi(argv[2]);
		return 1;
	} else {
		printf("[DEBUG] incorrect number of args\n");
		/* Incorrect number of args */
		return 0;
	}
}

void usage() {
	fprintf(stdout, "usage..\n");
	fprintf(stdout, "./$(PROGRAM) [-t | -u] [PORT]\n");
	fprintf(stdout, "		-t | -u : tcp or udp protocol (only one may be used)\n");
	fprintf(stdout, "		PORT : port number to be listened to\n");
}
