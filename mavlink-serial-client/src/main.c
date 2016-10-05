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
#include <sqlite3.h>

/* Common mavlink messages */
#include <mavlink.h>

/* Serial port settings */
#define BAUDRATE B57600
#define BUFFER_LEN 2041

#define TRUE 1
#define FALSE 0

/* Globals(termios and database specific) */
struct termios	oldtio, newtio;
int							fd;
char						database_file[50];
char						session_id[12];
sqlite3					*db;

/* Signal handler */
void signal_handler(int);
/* Start messaging procedure */
void sendMessages(); 
/* Insert records into database */
void savePersistantData(mavlink_message_t mavmsg, uint8_t* mavframe, 
		struct tm * timeinfo, int msg_size, double rtt, double uplink_time, 
		double downlink_time);
/* callback function from sqlite3 c api */
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	int i = 0;
	for (i=0; i<argc; i++) {
		fprintf(stdout, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	fprintf(stdout, "\n");
	return 0;
}

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
	 * Open serial port ttyUSB0 
	 * O_RDWR			= Open port with read / write
	 * ## default is a blocking read */
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
		// reset options and close stuff
		tcsetattr(fd, TCSANOW, &oldtio);
		close(fd);
		sqlite3_close(db);

		fprintf(stdout, "signal_handler: closing application..\n");
		exit(0);
	}
}

void sendMessages() {
	/* len	: function return values */
	int bytes_read, bytes_sent, len, i = 0;
	/* mavlink variables */
	mavlink_message_t mavmsg;
	mavlink_status_t mavstatus;
	/* buf	: sending buffer 
	 * temp	: temporary uint8_t value */
	uint8_t buf[BUFFER_LEN];
	uint8_t temp;
	/* flags */
	/* Timers and time */
	clock_t timer;
	double time_taken;
	time_t time_var = time(NULL);
	struct tm time_struct = *localtime(&time_var);

	/* Indicate test has started*/
	fprintf(stdout, "Starting CLIENT, session ID: %s, date: %d-%d-%d\n",
			session_id, 
			time_struct.tm_year+1900, time_struct.tm_mon+1, time_struct.tm_mday);

	/* Main loop */
	while(TRUE) {
		/* Reset buffers and variables */
		i = 0;
		bytes_read = 0;
		temp = 0;
		memset((char*)&mavstatus, 0, sizeof(mavstatus));
		memset((char*)&mavmsg, 0, sizeof(mavmsg));
		memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LEN);		// mavmsg buffer

		/* Create mavlink message: HEARTBEAT 
		 * TODO: custom mavlink messages */
		mavlink_msg_heartbeat_pack(1, 200, &mavmsg,
				MAV_TYPE_HELICOPTER,
				MAV_AUTOPILOT_GENERIC,
				MAV_MODE_GUIDED_ARMED, 0, MAV_STATE_ACTIVE);
		len = mavlink_msg_to_send_buffer(buf, &mavmsg);

		/* Send the message */
		if ((bytes_sent = write(fd, buf, len)) < 0 ) {
			fprintf(stderr, "Unable to send! exiting..\n");
			exit(1);
		} 

		/* Start timer */
		timer = clock();

		/* Read one byte from radio */ 
		while ((len = read(fd, &temp, 1)) > 0) {
			buf[i++] = temp;
			bytes_read += len;
			/* Parse packet */
			if (mavlink_parse_char(MAVLINK_COMM_0, temp, &mavmsg, &mavstatus)) {
				/* Valid packet received, switch on msgid */
				if (mavmsg.msgid == MAVLINK_MSG_ID_HEARTBEAT) {
					/* Stop timer */
					timer = clock() - timer;
					time_taken = ((double)timer) / CLOCKS_PER_SEC;
					time_taken = time_taken * 1000;

					/* Get current date */
					time_var = time(NULL);
					time_struct = *localtime(&time_var);

					/* Indicate packet is received */
					fprintf(stdout, "%d:%d:%d recv HEARTBEAT: size=%d seq=%d rtt=%lfms\n", 
							time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec, 
							bytes_read, mavmsg.seq, time_taken);
					fprintf(stdout, "Received packet: SYS:%d, COMP:%d, LEN:%d, MSG ID:%d\n",
							mavmsg.sysid, mavmsg.compid, mavmsg.len, mavmsg.msgid);

					/* Save data */
					savePersistantData(mavmsg, buf, &time_struct, bytes_read, time_taken, 0.0, 0.0);

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

					/* Clear buffers */
					i = 0;
					bytes_read = 0;
					temp = 0;
					memset((char*)&mavstatus, 0, sizeof(mavstatus));
					memset((char*)&mavmsg, 0, sizeof(mavmsg));
					memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LEN);
				} else {
					/* Clear buffers */
					i = 0;
					bytes_read = 0;
					temp = 0;
					memset((char*)&mavstatus, 0, sizeof(mavstatus));
					memset((char*)&mavmsg, 0, sizeof(mavmsg));
					memset((char*)buf, 0, sizeof(uint8_t) * BUFFER_LEN);
				}
			}
		}
	}
}

void savePersistantData(mavlink_message_t mavmsg, uint8_t* mavframe, 
		struct tm * timeinfo, int msg_size, double rtt, double uplink_time, 
		double downlink_time) {
	char insert_query[BUFFER_LEN];
	int i;
	char buf[1024];		// buffer for mav raw frame
	char buf_t[3];
	char timebuf[20];	// buffer for time
	char *err_msg = 0;
	memset(buf, 0, sizeof(char) * 1024);		// reset buffer

	/* Create SQL insert statement */
	for (i=0; i<msg_size; i++) {
		snprintf(buf_t, 3, "%02X", mavframe[i]);
		strcat(buf, buf_t);
	}
	strftime(timebuf, 20, "%X", timeinfo);
	snprintf(insert_query, BUFFER_LEN, "INSERT INTO records (session_id, frame_seq, frame_contents, time_sent, msg_size, rtt, uplink_time, downlink_time) VALUES ( '%s', %d, '%s', '%s', %d, %lf, %lf, %lf );", session_id, mavmsg.seq, buf, timebuf, msg_size, rtt, uplink_time, downlink_time);

	/* Execute SQL insert statement */
	if (sqlite3_exec(db, insert_query, callback, 0, &err_msg) != SQLITE_OK) {
		fprintf(stderr, "SQL ERROR: %s\n", err_msg);
		sqlite3_free(err_msg);
	}
}
