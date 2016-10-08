#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <time.h>
#include <sys/time.h>
#include "dbfunctions.h"
#include <mavlink.h>

#define DEST_MAC_ADDR "00:1A:7D:DA:71:05"

void sendMessages(int fd);

/*
 * argv[1] : DEST_MAC_ADDR 
 * argv[2] : DB_FILE */
int main(int argc, char **argv) {
	// Bluetooth specific variables
	struct sockaddr_l2 addr = { 0 };
	int fd, status;
	char dest[18];
	struct timeval timeout;

	/* Copy destination MAC address */
	strcpy(dest, DEST_MAC_ADDR);
	printf("%s\n", dest);

	/* allocate a socket */
	fd = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (fd < 0) {
		printf("Somethings up..\n");
		exit(1);
	}

	/* set the connection parameters (who to connect to) */
	addr.l2_family = AF_BLUETOOTH;
	addr.l2_psm = htobs(0x1001);
	str2ba( dest, &addr.l2_bdaddr );

	/* Set socket options for 5 second timeout */
	timeout.tv_sec = 5;
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, 
				(struct timeval *)&timeout, sizeof(struct timeval)) < 0) {
		fprintf(stderr, "Error in SO_RCVTIMEO\n");
		exit(1);
	}

	/* connct to server */
	status = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
	printf("Status: %d\n", status);

	/* Start sending messages */
	sendMessages(fd);
}

void sendMessages(int fd) {
	uint8_t bytes_read, bytes_sent, len, i = 0;
	mavlink_message_t mavmsg;
	mavlink_status_t status;
	uint8_t buf[BUFFER_LEN], recvbuf[BUFFER_LEN];
	uint8_t temp;
	int seq_num = 1;
	// Timer variables
	struct timeval tv;
	double timestamp_echo = 0.0, timestamp_cur = 0.0,
				 time_taken, uplink_time, downlink_time;
	time_t time_var = time(NULL);
	struct tm time_struct = *localtime(&time_var);

	/* Indicate test has started */
	fprintf(stdout, "Starting bluetooth client, date: %d-%d-%d\n",
			time_struct.tm_year + 1900, time_struct.tm_mon + 1, time_struct.tm_mday);

	/* Main messaging loop */
	while (1) {
		/* Reset buffers and variables */
		memset((char*)&mavmsg, 0, sizeof(mavmsg));
		memset((char*)&status, 0, sizeof(status));
		memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LEN);
		memset((char*)recvbuf, 0, sizeof(uint8_t) * BUFFER_LEN);

		/* Set mavlink message: HEARTBEAT */
		mavlink_msg_test_frame_pack(1, 200, &mavmsg,
				seq_num, timestamp_cur, timestamp_echo);
		len = mavlink_msg_to_send_buffer(buf, &mavmsg);
		seq_num++;

		/* Send the message */
		if ((bytes_sent = write(fd, buf, len)) < 0) {
			fprintf(stderr, "Unable to send! Exiting..\n");
			exit(1);
		}

		/* Receive a reply and print it */
		memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LEN);
		if ((bytes_read = read(fd, buf, sizeof(buf))) < 0) {
			fprintf(stderr, "Unable to receive sequence %u\n", seq_num - 1);
			/* Pack dummy mavlink message */
			mavlink_msg_test_frame_pack(1, 200, &mavmsg,
					seq_num - 1, -99.9, -99.9);
			len = mavlink_msg_to_send_buffer(buf, &mavmsg);
			/* Get local time */
			time_var = time(NULL);
			time_struct = *localtime(&time_var);
			savePersistantData(mavmsg, buf, &time_struct, len, -99.9, -99.9, -99.9);
		} else {
			/* Parse packet */
			for (i=0; i<bytes_read; i++) {
				if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &mavmsg, &status)) {
					/* Get measured rtt, uplink and downlink time */
					gettimeofday(&tv, NULL);
					timestamp_cur = ((double)(tv.tv_sec) * 1000) 
						+ ((double)(tv.tv_usec) / 1000);
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
					fprintf(stdout, "%d:%d:%d %d bytes(TEST_FRAME)\n",
							time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec, 
							(int)bytes_read);
					fprintf(stdout, "seq_num: %u rtt: %lf, ut: %lfms, dt: %lfms\n",
							mavlink_msg_test_frame_get_sequence(&mavmsg),
							time_taken, uplink_time, downlink_time);
					fprintf(stdout, "[DEBUG] timestamp_sender: %lf, timestamp_echo: %lf\n",
							mavlink_msg_test_frame_get_timestamp_sender(&mavmsg),
							mavlink_msg_test_frame_get_timestamp_echo(&mavmsg));

					/* Save data to database */
					savePersistantData(mavmsg, recvbuf, &time_struct, (int)bytes_read, 
							time_taken, uplink_time, downlink_time);

					/* Sleep for a second */
					sleep(1);
				} else if (i == (bytes_read-1)) {
					/* Broken packet */
					printf("%d:%d:%d Broken packet(Invalid checksom)\n", 
							time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec);
				}
			}
		}
	}
}
