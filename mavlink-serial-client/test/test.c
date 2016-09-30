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
	int bytes_sent, len, i=0;
	mavlink_message_t mavmsg;
	mavlink_status_t mavstatus;
	uint8_t buf[BUFFER_LEN];
	time_t time_var = time(NULL);
	struct tm time_struct = *localtime(&time_var);

	/* Indicate test has started*/
	fprintf(stdout, "Testing code, date: %d-%d-%d\n",
			time_struct.tm_year+1900, time_struct.tm_mon+1, time_struct.tm_mday);

	/* Main loop */
	while(TRUE) {
		/* Reset buffers and variables */
		memset((char*)&mavstatus, 0, sizeof(mavstatus));
		memset((char*)&mavmsg, 0, sizeof(mavmsg));
		memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LEN);		// Sending buffer

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
		} else {
			fprintf(stdout, "Sending packet: SYS:%d, COMP:%d, LEN:%d, MSG ID:%d\n",
								mavmsg.sysid, mavmsg.compid, mavmsg.len, mavmsg.msgid);
			fprintf(stdout, "Sent msg: ");
			for (i=0; i<len; i++) {
				fprintf(stdout, "%02X", (unsigned char)buf[i]);
			}
			fprintf(stdout, "\n");
		}

		/* sleep for 1 second */
		sleep(1);
	}
}
