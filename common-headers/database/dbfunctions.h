#ifndef _DBFUNCTIONS_H_
#define _DBFUNCTIONS_H_

#include <sqlite3.h>
#include <mavlink.h>

#define BUFFER_LEN 2041

/* GLOBALS */
char						database_file[50];
char						session_id[12];
sqlite3					*db;

/* callback function from sqlite3 c api */
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	int i = 0;
	for (i=0; i<argc; i++) {
		fprintf(stdout, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	fprintf(stdout, "\n");
	return 0;
}

/* Save mavlink messages to sqlite3 database */
void savePersistantData(mavlink_message_t mavmsg, uint8_t* mavframe, 
		const struct tm * timeinfo, int msg_size, double rtt, double uplink_time, 
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

#endif
