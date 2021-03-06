#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <termios.h>
#include <stdint.h>
#include <time.h>

/* Common mavlink messages */
#include <mavlink.h>

/* Serial port settings */
#define BAUDRATE B57600
#define BUFFER_LEN 2041

/* Globals */
struct termios	oldtio, newtio;
int							fd;

/* Signal handler */
void signal_handler(int);
/* Start messaging procedure */
void serverRoutine(); 

/*
 * main procedure for the serial client
 * argv[1] : device path */
int main(int argc, char **argv){
	if (argc != 2) {
		fprintf(stderr, "Fatal error! not enough args\n");
		exit(1);
	}

	/*
	 * O_RDWR			= Open port with read / write
	 * ## default is a blocking read */
	if ((fd = open(argv[1], O_RDWR)) == -1) {
		fprintf(stderr, "main: %s open failed!\n", argv[1]);
		exit(1);
	} else {
		fprintf(stdout, "main: using %s..\n", argv[1]);
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
	newtio.c_cc[VTIME] = 0;		// timer unused 
	newtio.c_cc[VMIN] = 1;		// min 1 character

	/* Flush modem */
	tcflush(fd, TCIFLUSH);

	/* Set with new terminal settings */
	tcsetattr(fd, TCSANOW, &newtio);
	fprintf(stdout, "main: opening done! fd = %d\n", fd);

	/* Register SIGINT signal handler */
	printf("main: registering signal..\n");
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		fprintf(stderr, "Cannot catch\n");
	}

	/* Start sending messages */
	serverRoutine();

	return 0;
}

/////////////////////////////////////// Function Definitions ///////////////////////

void signal_handler(int sign) {
	if (sign == SIGINT) {
		fprintf(stdout, "signal_handler: SIGINT\n");
		// reset options
		tcsetattr(fd, TCSANOW, &oldtio);
		close(fd);

		fprintf(stdout, "signal_handler: closing application..\n");
		exit(0);
	}
}

void serverRoutine() {
	uint32_t bytes_read, bytes_sent, len;
	mavlink_message_t mavmsg, newmsg;
	mavlink_status_t status;
	uint8_t buf[BUFFER_LEN];
	uint8_t temp;
	struct timeval tv;
	double timestamp = 0.0;
	time_t time_var = time(NULL);
	struct tm time_struct = *localtime(&time_var);

	/* Indicate test has started*/
	fprintf(stdout, "Starting main procedure for the server, date: %d-%d-%d\n",
			time_struct.tm_year+1900, time_struct.tm_mon+1, time_struct.tm_mday);

	/* Main loop */
	while(1) {
		/* Reset buffers and variables */
		bytes_read = 0;
		temp = 0;
		memset((char*)&status, 0, sizeof(status));
		memset((char*)&mavmsg, 0, sizeof(mavmsg));
		memset((char*)&newmsg, 0, sizeof(newmsg));
		memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LEN);		// Sending buffer

		/* Start reading bytes when min charater is received */
		while ((len = read(fd, &temp, 1)) > 0) {
			bytes_read += len;
			/* Parse packet */
			if (mavlink_parse_char(MAVLINK_COMM_0, temp, &mavmsg, &status)) {
				if (mavmsg.msgid == MAVLINK_MSG_ID_TEST_FRAME) {
					/* Get current timestamp */
					gettimeofday(&tv, NULL);
					timestamp = ((double)(tv.tv_sec) * 1000)
						+ ((double)(tv.tv_usec) / 1000);

					/* Indicate frame is received */
					fprintf(stdout, "%d bytes received\n", bytes_read);
					fprintf(stdout, "[DEBUG] seq_num: %u, timestamp_sender: %lf, timestamp_echo: %lf\n",
							mavlink_msg_test_frame_get_sequence(&mavmsg),
							mavlink_msg_test_frame_get_timestamp_sender(&mavmsg),
							mavlink_msg_test_frame_get_timestamp_echo(&mavmsg));

					/* Repack message with timestamp */
					mavlink_msg_test_frame_pack(1, 200, &newmsg, 
							mavlink_msg_test_frame_get_sequence(&mavmsg),
							mavlink_msg_test_frame_get_timestamp_sender(&mavmsg),
							timestamp);
					len = mavlink_msg_to_send_buffer(buf, &newmsg);

					/* Debug string */
					fprintf(stdout, "[DEBUG] seq_num: %u, timestamp_sender: %lf, timestamp_echo: %lf\n",
							mavlink_msg_test_frame_get_sequence(&newmsg),
							mavlink_msg_test_frame_get_timestamp_sender(&newmsg),
							mavlink_msg_test_frame_get_timestamp_echo(&newmsg));

					/* Echo back message */
					if ((bytes_sent = write(fd, buf, len)) < 0) {
						fprintf(stderr, "Unable to send! exiting..\n");
						exit(1);
					}

					/* Break reading loop */
					break;
				}
			}
		}
	}
}
