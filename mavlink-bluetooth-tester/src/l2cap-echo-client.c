#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <time.h>

#include <mavlink.h>

#define DEST_MAC_ADDR "00:1A:7D:DA:71:05"
#define BUFFER_LEN 1024

void sendMessages(int fd);

int main(int argc, char **argv) {
	// Bluetooth specific variables
	struct sockaddr_l2 addr = { 0 };
	int fd, status;
	char dest[18];

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

	/* connct to server */
	status = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
	printf("Status: %d\n", status);

	/* Start sending messages */
	sendMessages(fd);
}

void sendMessages(int fd) {
	// MAV message variables
	int i;
	int bytes_read, bytes_sent, len;
	mavlink_message_t mavmsg;
	mavlink_status_t mavstatus;
	uint8_t buf[BUFFER_LEN];
	uint8_t temp;
	int seq_num = 1;
	// Timer variables
	clock_t timer;
	double time_taken;
	time_t time_var = time(NULL);
	struct tm time_struct = *localtime(&time_var);

	/* Indicate test has started 
	 * TODO: Add date */
	printf("Sending HEARTBEAT(17 bytes), date:\n");

	/* send a message */
	while (1) {
		memset((char*)&mavmsg, 0, sizeof(mavmsg));
		memset((char*)&buf, 0, sizeof(uint8_t)*BUFFER_LEN);

		/* Set mavlink message: HEARTBEAT */
		mavlink_msg_heartbeat_pack(1, 200, &mavmsg, 
				MAV_TYPE_HELICOPTER, 
				MAV_AUTOPILOT_GENERIC, 
				MAV_MODE_GUIDED_ARMED, 0, MAV_STATE_ACTIVE);
		len = mavlink_msg_to_send_buffer(buf, &mavmsg);

		/* Send the message
		 * Measure round trip time */
		if ((bytes_sent = write(fd, buf, len)) < 0) {
			perror("Unable to send");
		}
		timer = clock();

		/* Receive a reply and print it */
		memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LEN);
		if ((bytes_read = read(fd, buf, sizeof(buf))) < 0) {
			perror("Unable to receive");
		} else {
			/* Zero-out mavmsg and status */
			memset((char*)&mavmsg, 0, sizeof(mavmsg));
			memset((char*)&mavstatus, 0, sizeof(mavstatus));

			/* Parse packet */
			for (i=0; i<bytes_read; i++) {
				temp = buf[i];
				if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &mavmsg, &mavstatus)) {
					/* Stop timer */
					timer = clock() - timer;
					time_taken = ((double)timer) / CLOCKS_PER_SEC; // in seconds
					time_taken = time_taken * 1000; // in milliseconds

					/* Get local time */
					time_var = time(NULL);
					time_struct = *localtime(&time_var);

					/* Indicate packet is received */
					printf("%d:%d:%d %d bytes from MAV(%s): seq=%d rtt=%lfms\n",
							time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec, bytes_read,
							DEST_MAC_ADDR, seq_num, time_taken);
					printf("Received packet: SYS:%d, COMP:%d, LEN:%d, MSG ID:%d\n",
							mavmsg.sysid, mavmsg.compid, mavmsg.len, mavmsg.msgid);
				} else if (i == (bytes_read-1)) {
					/* Broken packet */
					printf("%d:%d:%d Broken packet(Invalid checksom)\n", 
							time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec);
				}
			}
		}
		seq_num++;
		sleep(1);
	}
	close(fd);
}
