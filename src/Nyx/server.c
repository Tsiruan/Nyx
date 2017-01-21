#include "Nyx.h"


void Nyx_event_process_admin() {
	char buffer_stdin[BUFFER_SIZE];
	scanf("%s", buffer_stdin);

	if (strcmp(buffer_stdin, "shutdown") == 0 || strcmp(buffer_stdin, "sd") == 0) {
		Nyx_server_cleanup();
		Nyx_close();
		exit(0);
	}
}

void Nyx_event_process_client(int clientfd) {
	packet_elem_t buffer[BUFFER_SIZE];
	int readbyte;
	readbyte = read(clientfd, buffer, sizeof(buffer));

	if (readbyte < 0)	{perror("read() error"); exit(1);}
	else if (readbyte == 0) {
		printf("client disconnected!\n");
		Nyx_client_close(clientfd);
	} else {
		printf("msg_recv: [ %s / %s ]\n", DECODE_STATE(buffer[0]), DECODE_CMD(buffer[1]));	// debug

		cmd_t sent_cmd;

		Nyx_state_sync_check(clientfd, EXTRACT_STATE(buffer));
		Nyx_state_forward(clientfd, EXTRACT_CMD(buffer));
		sent_cmd = Nyx_state_exec(clientfd, EXTRACT_CMD(buffer), EXTRACT_CONTENT(buffer));
		Nyx_state_forward(clientfd, sent_cmd);
		//printf("%s\n", buffer);
	}
}


void Nyx_event_process(int eventfd) {
	if (eventfd == 0) {
		Nyx_event_process_admin();
	} else if (eventfd > 0) {
		Nyx_event_process_client(eventfd);
	}	
}

static void Nyx_readfds_init(fd_set *readfds) {
	FD_ZERO(readfds);
	FD_SET(0, readfds);
	Nyx_fdset_push_listenfd(readfds);
	Nyx_fdset_push_onlineusers(readfds);
}

int main(void) {
	Nyx_init();
	Nyx_listen();

	fd_set readfds;
	while(1) {
		Nyx_readfds_init(&readfds);

		int eventfd = Nyx_select(&readfds);
		if (eventfd == NYX_SELECT_LISTEN) {
			Nyx_accept();
		} else {
			Nyx_event_process(eventfd);
		}
	}

	Nyx_close();
	return 0;
}