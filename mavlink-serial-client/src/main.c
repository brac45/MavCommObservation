#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <termios.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <mavlink.h>
#include "dbfunctions.h"

/* Serial port settings */
#define BAUDRATE B57600

/* Globals(termios specific) */
struct termios	oldtio, newtio;
int							fd;

/* Signal handler */
void signal_handler(int);
/* Start messaging procedure */
void sendMessages(); 

/*
 * main procedure for the serial client
 * argv[1] : device path 
 * argv[2] : sqlite3 database file path 
 * argv[3] : observation session ID */
int main(int argc, char **argv) {
	if (argc != 4) {
		fprintf(stderr, "usage..\n");
		fprintf(stderr, "		./$(PROGRAM) [device_path] [db path] [session id]\n");
		exit(1);
	}

	/*
	 * O_RDWR			= Open port with read | write */
	if ((fd = open(argv[1], O_RDWR)) == -1) {
		fprintf(stderr, "main: %s open failed!\n", argv[1]);
		exit(1);
	} else {
		fprintf(stdout, "main: using %s..\n", argv[1]);
	}

	/* Insert session id */
	strcpy(session_id, argv[3]);

	/* Copy sqlite3 database file path and try to open it */
	strcpy(database_file, argv[2]);
	if (sqlite3_open(database_file, &db)) {
		fprintf(stderr, "Unable to open %s %s\n", database_file, sqlite3_errmsg(db));
	} else {
		fprintf(stdout, "Opened DB %s\n", database_file);
	}

	/* Save current port settings */
	tcgetattr(fd, &oldtio);
	bzero(&newtio, sizeof(newtio));

	/*
	 * BAUDRATE = 57600
	 * CS8			= 8bit no parity 1stopbit
	 * CREAD		= enable receiver
	 * c_lflag	= 0 for non-canonical input
	 * VTIME		= inter-character timer
	 * VMIN			= blocking read until min char recv */
	newtio.c_cflag = BAUDRATE | CS8 | CREAD;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 50;		// timer for 5 seconds
	newtio.c_cc[VMIN] = 0;		// min 1 character

	/* Flush modem */
	tcflush(fd, TCIFLUSH);

	/* Set with new terminal settings */
	tcsetattr(fd, TCSANOW, &newtio);
	fprintf(stdout, "main: opening done! fd = %d\n", fd);

	/* Register SIGINT signal handler */
	printf("main: registering signal..\n");
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		fprintf(stderr, "Cannot catch\n");
		exit(1);
	}

	/* Start sending messages */
	sendMessages();

	return 0;
}

/////////////////////////////////////// Function Definitions ///////////////////////

void signal_handler(int sign) {
	if (sign == SIGINT) {
		fprintf(stdout, "signal_handler: SIGINT\n");
		// reset options and close stuff
		tcsetattr(fd, TCSANOW, &oldtio);
		close(fd);
		sqlite3_close(db);

		fprintf(stdout, "signal_handler: closing application..\n");
		exit(0);
	}
}

void sendMessages() {
	int bytes_read, bytes_sent, len, i = 0;
	uint32_t seq_num = 1;
	mavlink_message_t mavmsg;
	mavlink_status_t status;
	uint8_t buf[BUFFER_LEN], recvbuf[BUFFER_LEN];
	uint8_t temp;
	/* Timers and time */
	struct timeval tv;
	double timestamp_echo = 0.0, timestamp_cur = 0.0,
				 time_taken, uplink_time, downlink_time;
	time_t time_var = time(NULL);
	struct tm time_struct = *localtime(&time_var);

	/* Indicate test has started*/
	fprintf(stdout, "Starting CLIENT, session ID: %s, date: %d-%d-%d\n",
			session_id, 
			time_struct.tm_year+1900, time_struct.tm_mon+1, time_struct.tm_mday);

	/* Main loop */
	while(1) {
		/* Reset buffers and variables */
		i = 0;
		bytes_read = 0;
		temp = 0;
		memset((char*)&status, 0, sizeof(status));
		memset((char*)&mavmsg, 0, sizeof(mavmsg));
		memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LEN);
		memset((char*)recvbuf, 0, sizeof(uint8_t) * BUFFER_LEN);

		/* Get timestamp (milliseconds from epoch) */
		gettimeofday(&tv, NULL);
		timestamp_cur = ((double)(tv.tv_sec) * 1000) 
			+ ((double)(tv.tv_usec) / 1000);

		/* Create mavlink message: Test */
		mavlink_msg_test_frame_pack(1, 200, &mavmsg,
				seq_num, timestamp_cur, timestamp_echo);
		len = mavlink_msg_to_send_buffer(buf, &mavmsg);
		seq_num++;

		/* Send the message */
		if ((bytes_sent = write(fd, buf, len)) < 0 ) {
			fprintf(stderr, "Unable to send! exiting..\n");
			exit(1);
		} 

		/* Try to read bytes from radio */ 
		while ((len = read(fd, &temp, 1)) > 0) {
			recvbuf[i++] = temp;
			bytes_read += len;
			/* Parse packet */
			if (mavlink_parse_char(MAVLINK_COMM_0, temp, &mavmsg, &status)) {
				if (mavmsg.msgid == MAVLINK_MSG_ID_TEST_FRAME) {
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
					fprintf(stdout, "%d:%d:%d %d bytes recv(TEST_FRAME)\n", 
							time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec, 
							bytes_read);
					fprintf(stdout, "seq_num: %u rtt: %lf, ut: %lfms, dt: %lfms\n",
							mavlink_msg_test_frame_get_sequence(&mavmsg),
							time_taken, uplink_time, downlink_time);
					fprintf(stdout, "[DEBUG] start  : %lf\n",
							mavlink_msg_test_frame_get_timestamp_sender(&mavmsg));
					fprintf(stdout, "[DEBUG] echo   : %lf\n", 
							mavlink_msg_test_frame_get_timestamp_echo(&mavmsg));
					fprintf(stdout, "[DEBUG] current: %lf\n", 
							timestamp_cur);


					/* Save data */
					savePersistantData(mavmsg, recvbuf, &time_struct, bytes_read, 
							time_taken, uplink_time, downlink_time);

					/* Wait 1 second and break from read loop */
					sleep(1);
					break;
				} else if (mavmsg.msgid == MAVLINK_MSG_ID_RADIO_STATUS) {
					/* Get current date */
					time_var = time(NULL);
					time_struct = *localtime(&time_var);

					/* Show radio status */
					fprintf(stdout, "%d:%d:%d %d bytes from radio\n",
							time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec, 
							bytes_read);
					fprintf(stdout, "rssi: %u, remrssi: %u\n",
							mavlink_msg_radio_status_get_rssi(&mavmsg),
							mavlink_msg_radio_status_get_remrssi(&mavmsg));

					/* Clear buffers (Keep reading bytes) */
					i = 0;
					bytes_read = 0;
					temp = 0;
					memset((char*)&status, 0, sizeof(status));
					memset((char*)&mavmsg, 0, sizeof(mavmsg));
					memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LEN);
				} else {
					/* Clear buffers(Keep readig bytes) */
					i = 0;
					bytes_read = 0;
					temp = 0;
					memset((char*)&status, 0, sizeof(status));
					memset((char*)&mavmsg, 0, sizeof(mavmsg));
					memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LEN);
				}
			}
		}

		/* Unable to receive data in predefined time frame */
		if (len <= 0) {
			fprintf(stderr, "Unable to receive sequence %u\n", seq_num-1);
			/* Pack dummy mavlink message */
			mavlink_msg_test_frame_pack(1, 200, &mavmsg,
					seq_num - 1, -99.9, -99.9);
			len = mavlink_msg_to_send_buffer(buf, &mavmsg);
			/* Get local time */
			time_var = time(NULL);
			time_struct = *localtime(&time_var);
			savePersistantData(mavmsg, buf, &time_struct, len, -99.9, -99.9, -99.9);
		}
	}
}

