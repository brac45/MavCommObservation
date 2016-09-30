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

/* Common mavlink messages */
#include <mavlink.h>

/* Serial port settings */
#define BAUDRATE B57600
#define BUFFER_LEN 2041

#define TRUE 1
#define FALSE 0

/* Globals */
struct termios	oldtio, newtio;
int							fd;

/* Signal handler */
void signal_handler(int);
/* Start messaging procedure */
void sendMessages(); 

/*
 * main procedure for the serial client
 * argv[1] : device path */
int main(int argc, char **argv){
	if (argc != 2) {
		fprintf(stderr, "Fatal error! not enough args\n");
		exit(1);
	}

	/*
	 * Open serial port ttyUSB0 
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
	sendMessages();

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

void sendMessages() {
	/*
	 * len	: function return values */
	int bytes_read, bytes_sent, len;
	/* mavlink variables */
	mavlink_message_t mavmsg;
	mavlink_status_t mavstatus;
	/*
	 * buf	: sending buffer 
	 * temp	: temporary uint8_t value */
	uint8_t buf[BUFFER_LEN];
	uint8_t temp;
	/* flags */
	int should_resend_message = TRUE;
	int should_reset_buffers = TRUE;
	clock_t timer;
	double time_taken;
	time_t time_var = time(NULL);
	struct tm time_struct = *localtime(&time_var);

	/* Indicate test has started*/
	fprintf(stdout, "Starting CLIENT, date: %d-%d-%d\n",
			time_struct.tm_year+1900, time_struct.tm_mon+1, time_struct.tm_mday);

	/* Main loop */
	while(TRUE) {
		if (should_reset_buffers) {
			/* Reset buffers and variables */
			bytes_read = 0;
			temp = 0;
			memset((char*)&mavstatus, 0, sizeof(mavstatus));
			memset((char*)&mavmsg, 0, sizeof(mavmsg));
			memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LEN);		// Sending buffer
			should_reset_buffers = FALSE;
		}
		if (should_resend_message) {
			/* Set mavlink message: HEARTBEAT 
			 * TODO: custom mavlink messages */
			mavlink_msg_heartbeat_pack(1, 200, &mavmsg,
					MAV_TYPE_HELICOPTER,
					MAV_AUTOPILOT_GENERIC,
					MAV_MODE_GUIDED_ARMED, 0, MAV_STATE_ACTIVE);
			len = mavlink_msg_to_send_buffer(buf, &mavmsg);

			/* Send the message, measure rtt */
			if ((bytes_sent = write(fd, buf, len)) < 0 ) {
				fprintf(stderr, "Unable to send! exiting..\n");
				exit(1);
			} 

			/* Set flag for resending messages */
			should_resend_message = FALSE;

			/* Start timer */
			timer = clock();
		}
		/* Read one byte from buffer */
		while ((len = read(fd, &temp, 1)) > 0) {
			bytes_read += len;
			/* Parse packet */
			if (mavlink_parse_char(MAVLINK_COMM_0, temp, &mavmsg, &mavstatus)) {
				/* Valid packet received */
				switch (mavmsg.msgid) {
					/* Heartbeat message */
					case MAVLINK_MSG_ID_HEARTBEAT:
						/* Stop timer */
						timer = clock() - timer;
						time_taken = ((double)timer) / CLOCKS_PER_SEC;
						time_taken = time_taken * 1000;

						/* Get current date */
						time_var = time(NULL);
						time_struct = *localtime(&time_var);

						/* Indicate packet is received */
						should_reset_buffers = TRUE;
						should_resend_message = TRUE;
						fprintf(stdout, "%d:%d:%d %d bytes from MAV(HEARTBEAT): seq=-999 rtt%lfms\n", 
								time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec, 
								bytes_read, time_taken);
						fprintf(stdout, "Received packet: SYS:%d, COMP:%d, LEN:%d, MSG ID:%d\n",
								mavmsg.sysid, mavmsg.compid, mavmsg.len, mavmsg.msgid);
						break;

						/* Radio status message (sik radios only) */
					case MAVLINK_MSG_ID_RADIO_STATUS:
						/* Show radio status */
						should_reset_buffers = TRUE;
						should_resend_message = FALSE;
						fprintf(stdout, "%d:%d:%d %d bytes from radio\n",
								time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec, 
								bytes_read);
						fprintf(stdout, "rssi: %u, remrssi: %u\n",
								mavlink_msg_radio_status_get_rssi(&mavmsg),
								mavlink_msg_radio_status_get_remrssi(&mavmsg));
						break;
				}
				/* Break read loop */
				break;
			}
		}

		/* TODO: find a way to keep persistant data(MYSQL, sqlite3 etc) 
		 * TODO: What I should be testing:
		 *				- number of bytes sent/received 
		 *				- rtt(round trip time)
		 *				- uplink time
		 *				- downlink time 
		 *				- rssi
		 *				- remote rssi */

		/* sleep for 1 second */
		sleep(1);
	}
}
