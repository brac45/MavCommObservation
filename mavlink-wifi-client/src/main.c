#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
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

	/* Fill out addr */
	memset((char*)&remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port_num);
	if (inet_aton(target_ip, &remote.sin_addr) == 0) {
		perror("inet_aton() failed");
		exit(1);
	}

	/* Indicate that test has started */
	if (protocol == TCP) {
		fprintf(stdout, "Not yet implemented\n");
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
	clock_t timer;
	double time_taken;
	/* MAVLink message variables */
	mavlink_message_t mavmsg;
	mavlink_status_t status;
	uint8_t buf[BUFFER_LENGTH];
	ssize_t recvsize;
	int bytes_sent;
	int i, len;
	/* Time variables */
	time_t time_var = time(NULL);
	struct tm time_struct = *localtime(&time_var);
	
	unsigned int s_size = sizeof(*remote);
	int seq_num = 1;

	/* Main loop */
	while(1) {
		/* Reset buffers */
		memset((char*)&mavmsg, 0, sizeof(mavmsg));
		memset((char*)&status, 0, sizeof(status));
		memset((char*)buf, '\0', sizeof(uint8_t) * BUFFER_LENGTH);

		/* Set mavlink message : HEARTBEAT */
		mavlink_msg_heartbeat_pack(1, 200, &mavmsg, 
				MAV_TYPE_HELICOPTER, 
				MAV_AUTOPILOT_GENERIC, 
				MAV_MODE_GUIDED_ARMED, 0, MAV_STATE_ACTIVE);
		len = mavlink_msg_to_send_buffer(buf, &mavmsg);

		/* Send the message 
		 * Measure round trip time */
		if ((bytes_sent = sendto(fd, buf, len, 0, (struct sockaddr *)remote, s_size)) == -1) {
			perror("Unable to send");
		}
		timer = clock();

		/* Receive a reply and print it */
		memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LENGTH);
		if ((recvsize = recvfrom(fd, buf, BUFFER_LENGTH, 0, (struct sockaddr *)remote, &s_size)) < 0) {
			fprintf(stderr, "Unable to receive\n");
		} else {
			/* Zero-out mavmsg and status */
			memset((char*)&mavmsg, 0, sizeof(mavmsg));
			memset((char*)&status, 0, sizeof(status));

			/* Parse packet */
			for (i = 0; i < recvsize; i++) {
				if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &mavmsg, &status)) {
					/* Stop timer */
					timer = clock() - timer;
					time_taken = ((double)timer) / CLOCKS_PER_SEC; // in seconds
					time_taken = time_taken * 1000; // in milliseconds 

					/* Get local time */
					time_var = time(NULL);
					time_struct = *localtime(&time_var);

					/* Indicate packet is received */
					fprintf(stdout, "%d:%d:%d %d bytes from MAV(%s:%d): seq=%d rtt=%lfms\n",
							time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec, (int)recvsize,
							inet_ntoa(remote->sin_addr), ntohs(remote->sin_port),
							seq_num, time_taken);
					fprintf(stdout, "Received packet: SYS: %d, COMP: %d, LEN: %d, MSG ID: %d\n", mavmsg.sysid, mavmsg.compid, mavmsg.len, mavmsg.msgid);
				} else if (i == (recvsize-1)){
					/* Broken packet */
					fprintf(stdout, "%d:%d:%d Broken packet(Invalid checksum)\n", time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec);
				}
			}
			seq_num++;
		}

		fflush(stdout);
		sleep(1);
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
