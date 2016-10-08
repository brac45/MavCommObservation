#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <mavlink.h>

#define BUFFER_LENGTH 1024

/* Main server routine */
void serverRoutine(int);

int main(int argc, char **argv) {
	struct sockaddr_l2 loc_addr = { 0 }, rem_addr = { 0 };
	char buf[BUFFER_LENGTH] = { 0 };
	int s, client;
	socklen_t opt = sizeof(rem_addr);

	/* allocate a socket */
	s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

	// bind socket to port 0x1001 of the first available 
	// bluetooth adapter
	loc_addr.l2_family = AF_BLUETOOTH;
	loc_addr.l2_bdaddr = *BDADDR_ANY;
	loc_addr.l2_psm = htobs(0x1001);

	bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

	// put socket into listening mode
	listen(s, 1);

	// accept one connection
	client = accept(s, (struct sockaddr *)&rem_addr, &opt);

	ba2str( &rem_addr.l2_bdaddr, buf );
	fprintf(stdout, "accepted connection from %s\n", buf);

	/*
	while (1) {
		memset(buf, 0, sizeof(buf));

		printf("Reading data from client..\n");
		bytes_read = read(client, buf, sizeof(buf));
		if( bytes_read > 0 ) {
			printf("received [%s]\n", buf);

			printf("echoing data: %s .. fd=%d\n", buf, client);
			write(client, buf, bytes_read); 
		} else {
			printf("received none\n");
			break;
		}
	}
	*/

	/* Main server routine */
	serverRoutine(client);

	// close connection
	close(client);
	close(s);
}

void serverRoutine(int sock) {
	unsigned int i = 0;
	mavlink_message_t mavmsg, newmsg;
	mavlink_status_t status;
	struct timeval tv;
	int recv_len, retlen;
	uint8_t buf[BUFFER_LENGTH];
	double timestamp;

	while(1) {
		memset((char*)buf, 0, BUFFER_LENGTH);
		memset((char*)&mavmsg, 0, sizeof(mavmsg));
		memset((char*)&newmsg, 0, sizeof(newmsg));
		memset((char*)&status, 0, sizeof(status));

		fprintf(stdout, "Waiting for messages..\n");

		if ((recv_len = read(sock, buf, BUFFER_LENGTH))) {
			fprintf(stderr, "read failed for some reason\n");
		} else {
			/* Parse packet */
			for (i=0; i<recv_len; i++) {
				if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &mavmsg, &status)) {
					/* Indicate frame is received */
					/* Indicate packet is received */
					fprintf(stdout, "%d bytes received\n",
							(int)recv_len);
					fprintf(stdout, "[DEBUG] seq_num: %u, timestamp_sender: %lf, timestamp_echo: %lf\n",
							mavlink_msg_test_frame_get_sequence(&mavmsg),
							mavlink_msg_test_frame_get_timestamp_sender(&mavmsg),
							mavlink_msg_test_frame_get_timestamp_echo(&mavmsg));

					/* Get current timestamp */
					gettimeofday(&tv, NULL);
					timestamp = ((double)(tv.tv_sec) * 1000) 
						+ ((double)(tv.tv_usec) / 1000);

					/* Reset buffer */
					memset((char*)buf, 0, BUFFER_LENGTH);

					/* Repack message with timestamp */
					mavlink_msg_test_frame_pack(1, 200, &newmsg, 
							mavlink_msg_test_frame_get_sequence(&mavmsg),
							mavlink_msg_test_frame_get_timestamp_sender(&mavmsg),
							timestamp);
					retlen = mavlink_msg_to_send_buffer(buf, &newmsg);

					/* Debug string */
					fprintf(stdout, "[DEBUG] seq_num: %u, timestamp_sender: %lf, timestamp_echo: %lf\n",
							mavlink_msg_test_frame_get_sequence(&newmsg),
							mavlink_msg_test_frame_get_timestamp_sender(&newmsg),
							mavlink_msg_test_frame_get_timestamp_echo(&newmsg));

					/* Echo back */
					write(sock, buf, recv_len);
				}
			}
		}
	}
}
