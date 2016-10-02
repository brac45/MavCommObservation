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
#define PORT_NUM			8888 // Port number

/* Start sending messages */
void sendMessages(int fd, struct sockaddr_in* remote);

/* Calculate length of the mavlink message */

/* Main function */
int main(int argc, char* argv[]) {
	struct sockaddr_in remote;
	char target_ip[50];
	int fd;
	int i;
	int len;

	/* First argv is the ip of the server */
	if (argc == 2) {
		strcpy(target_ip, argv[1]);
		printf("Target IP: %s\n", target_ip);
	} else {
		printf("Enter correct args\n");
		exit(1);
	}

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
	remote.sin_port = htons(PORT_NUM);
	if (inet_aton(target_ip, &remote.sin_addr) == 0) {
		perror("inet_aton() failed");
		exit(1);
	}

	sendMessages(fd, &remote);
}

void sendMessages(int fd, struct sockaddr_in* remote) {
	clock_t timer;
	double time_taken;
	mavlink_message_t mavmsg;
	mavlink_status_t status;
	uint8_t buf[BUFFER_LENGTH];
	ssize_t recvsize;
	int bytes_sent;
	uint16_t len;
	int i;
	unsigned int temp;
	time_t time_var = time(NULL);
	struct tm time_struct = *localtime(&time_var);
	int s_size = sizeof(*remote);
	int seq_num = 1;

	/* Indicate that test has started */
	printf("Sending HEARTBEAT(17 bytes), date: %d-%d-%d\n", 
			time_struct.tm_year + 1900, time_struct.tm_mon + 1, time_struct.tm_mday);

	while(1) {
		memset((char*)&mavmsg, 0, sizeof(mavmsg));
		memset((char*)buf, '\0', sizeof(uint8_t) * BUFFER_LENGTH);

		/* Set mavlink message : HEARTBEAT */
		mavlink_msg_heartbeat_pack(1, 200, &mavmsg, MAV_TYPE_HELICOPTER, MAV_AUTOPILOT_GENERIC, MAV_MODE_GUIDED_ARMED, 0, MAV_STATE_ACTIVE);
		len = mavlink_msg_to_send_buffer(buf, &mavmsg);

		/* Send the message 
		 * Measure round trip time */
		if ((bytes_sent = sendto(fd, buf, len, 0, (struct sockaddr *)remote, s_size)) == -1) {
			perror("Unable to send");
		}
		timer = clock();

		/* Receive a reply and print it */
		memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LENGTH);
		if ((recvsize = recvfrom(fd, buf, BUFFER_LENGTH, 0, (struct sockaddr *)remote, &s_size)) == -1) {
			perror("Unable to receive");
		} else {
			/* Zero-out mavmsg and status */
			memset((char*)&mavmsg, 0, sizeof(mavmsg));
			memset((char*)&status, 0, sizeof(status));

			/* Parse packet */
			for (i = 0; i < recvsize; i++) {
				temp = buf[i];
				if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &mavmsg, &status)) {
					/* Stop timer */
					timer = clock() - timer;
					time_taken = ((double)timer) / CLOCKS_PER_SEC; // in seconds
					time_taken = time_taken * 1000; // in milliseconds 

					/* Get local time */
					time_var = time(NULL);
					time_struct = *localtime(&time_var);

					/* Indicate packet is received */
					printf("%d:%d:%d %d bytes from MAV(%s:%d): seq=%d rtt=%lfms\n",
							time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec, (int)recvsize,
							inet_ntoa(remote->sin_addr), ntohs(remote->sin_port),
							seq_num, time_taken);
					printf("Received packet: SYS: %d, COMP: %d, LEN: %d, MSG ID: %d\n", mavmsg.sysid, mavmsg.compid, mavmsg.len, mavmsg.msgid);
				} else if (i == (recvsize-1)){
					/* Broken packet */
					printf("%d:%d:%d Broken packet(Invalid checksum)\n", time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec);
				}
			}
			seq_num++;
		}

		fflush(stdout);
		sleep(1);
	}
}
