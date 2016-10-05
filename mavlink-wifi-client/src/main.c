#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

/* This assumes you have the mavlink headers on your include path
	 or in the same folder as this source file */
#include <mavlink.h>

/* Constants */
#define BUFFER_LENGTH 2041 
#define TCP 99
#define UDP 98
#define UNDEFINED 999

/* Start sending messages using UDP
 * @args int		file descriptor
 *			 struct sockaddr_in* remote address */
void sendMessagesUDP(int, struct sockaddr_in*);
/* Start sending messages using TCP
 * @args int		file descriptor
 *			 struct sockaddr_in* remote address */
void sendMessagesTCP(int, struct sockaddr_in*);
/* Parse args 
 * @args int		argc
 *       char** argv
 *       int	  protocol
 *       char*	target_ip 
 *       int*		port_num
 * @return 1 if successful, 0 if not */
int parseArgs(int, char**, int*, char*, int*);
/* Show usage */
void usage();

/* Main function */
int main(int argc, char* argv[]) {
	struct sockaddr_in remote;
	char target_ip[50];
	int port_num = 0;
	int protocol = UNDEFINED;
	int fd;
	/* Time variables */
	time_t time_var = time(NULL);
	struct tm time_struct = *localtime(&time_var);

	/* Parse arguments */
	if (!parseArgs(argc, argv, &protocol, target_ip, &port_num)) {
		usage();
		exit(1);
	}

	/* Display input arguments */
	fprintf(stdout, "[DEBUG] protocol : %d, ip : %s, port : %d\n", 
			protocol, target_ip, port_num);

	/* Create a socket for sending datagrams
	 * IP protocol family: AF_INET
	 * UDP protocol: SOCK_DGRAM */
	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("Error while creating socket(main)");
		exit(1);
	}

	/* Fill out addresses */
	memset((char*)&remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port_num);
	if (inet_aton(target_ip, &remote.sin_addr) == 0) {
		perror("inet_aton() failed");
		exit(1);
	}

	/* Start test */
	if (protocol == TCP) {
		fprintf(stdout, "TCP not yet implemented\n");
		exit(1);
	} else if (protocol == UDP) {
		fprintf(stdout, "Starting wifi UDP client, date: %d-%d-%d\n", 
				time_struct.tm_year + 1900, time_struct.tm_mon + 1, time_struct.tm_mday);
		sendMessagesUDP(fd, &remote);
	} else {
		usage();
		exit(1);
	}
}

void sendMessagesUDP(int fd, struct sockaddr_in* remote) {
	/* MAVLink message variables */
	mavlink_message_t mavmsg;
	mavlink_status_t status;
	uint8_t buf[BUFFER_LENGTH];
	ssize_t recvsize;
	int bytes_sent;
	int i, len;
	/* Time variables */
	struct timeval tv;
	uint64_t timestamp_echo = 0, timestamp_cur = 0, 
					 time_taken, uplink_time, downlink_time;
	time_t time_var = time(NULL);
	struct tm time_struct = *localtime(&time_var);
	unsigned int s_size = sizeof(*remote);
	uint32_t seq_num = 1;

	/* Main loop */
	while(1) {
		/* Reset buffers */
		memset((char*)&mavmsg, 0, sizeof(mavmsg));
		memset((char*)&status, 0, sizeof(status));
		memset((char*)buf, '\0', sizeof(uint8_t) * BUFFER_LENGTH);

		/* Get timestamp (milliseconds from epoch) */
		gettimeofday(&tv, NULL);
		timestamp_cur = ((uint64_t)(tv.tv_sec) * 1000) 
			+ ((uint64_t)(tv.tv_usec) / 1000);

		/* Pack mavlink message */
		mavlink_msg_test_frame_pack(1, 200, &mavmsg,
				seq_num, timestamp_cur, timestamp_echo);
		len = mavlink_msg_to_send_buffer(buf, &mavmsg);
		seq_num++;

		/* Send the message */
		if ((bytes_sent = sendto(fd, buf, len, 0, (struct sockaddr *)remote, s_size)) == -1) {
			perror("Unable to send");
		}

		/* Reset buffers and variables */
		memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LENGTH);
		memset((char*)&mavmsg, 0, sizeof(mavmsg));
		memset((char*)&status, 0, sizeof(status));

		/* Receive a datagram */
		if ((recvsize = recvfrom(fd, buf, BUFFER_LENGTH, 0, (struct sockaddr *)remote, &s_size)) < 0) {
			fprintf(stderr, "Unable to receive\n");
		} else {
			/* Parse packet */
			for (i = 0; i < recvsize; i++) {
				if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &mavmsg, &status)) {
					if (mavmsg.msgid == MAVLINK_MSG_ID_TEST_FRAME) {
						/* Get measured rtt, uplink and downlink time */
						gettimeofday(&tv, NULL);
						timestamp_cur = ((uint64_t)(tv.tv_sec) * 1000) 
							+ ((uint64_t)(tv.tv_usec) / 1000);
						time_taken = timestamp_cur - 
							mavlink_msg_test_frame_get_timestamp_sender(&mavmsg);
						uplink_time = mavlink_msg_test_frame_get_timestamp_echo(&mavmsg) -
							mavlink_msg_test_frame_get_timestamp_sender(&mavmsg);
						downlink_time = timestamp_cur - 
							mavlink_msg_test_frame_get_timestamp_echo(&mavmsg);

						/* Get local time */
						time_var = time(NULL);
						time_struct = *localtime(&time_var);

						/* Indicate packet is received */
						fprintf(stdout, "%d:%d:%d %d bytes from MAV(%s:%d)\n",
								time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec, 
								(int)recvsize, inet_ntoa(remote->sin_addr), ntohs(remote->sin_port));
						fprintf(stdout, "seq_num: %u rtt: %lu, ut: %lums, dt: %lums\n",
								mavlink_msg_test_frame_get_sequence(&mavmsg),
								time_taken, uplink_time, downlink_time);
						fprintf(stdout, "[DEBUG] timestamp_sender: %lu, timestamp_echo: %lu\n",
								mavlink_msg_test_frame_get_timestamp_sender(&mavmsg),
								mavlink_msg_test_frame_get_timestamp_echo(&mavmsg));

						/* sleep for a second */
						sleep(1);
					}
				} 
			}
		}
	}
}

int parseArgs(int argc, char** argv, int* protocol, char* target_ip, int* port_num) {
	/* TCP or UDP */
	if (argc == 6) {
		/* TCP or UDP */
		if (strcmp("-t", argv[1]) == 0) {
			*protocol = TCP;
		} else if (strcmp("-u", argv[1]) == 0) {
			*protocol = UDP;
		} else {
			return 0;
		}

		/* IP address */
		if (strcmp("--addr", argv[2]) == 0) {
			strcpy(target_ip, argv[3]);
		} else {
			return 0;
		}

		/* Port number*/
		if (strcmp("--port", argv[4]) == 0) {
			*port_num = atoi(argv[5]);
			return 1;
		} else {
			printf("[DEBUG] incorrect argv[4]\n");
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
	fprintf(stdout, "./$(PROGRAM) [-t | -u] [--addr ADDRESS] [--port PORT]\n");
	fprintf(stdout, "		-t | -u : tcp or udp protocol (only one may be used)\n");
	fprintf(stdout, "		--addr : ipv4 address of the target\n");
	fprintf(stdout, "		--port : port number of the target\n");
}
