#include "Nyx.h"

static void task_from_client(int eventfd) {
	packet_elem_t buffer[BUFFER_SIZE];
	int readbyte;
	readbyte = read(eventfd, buffer, sizeof(buffer));

	if (readbyte < 0)	{perror("read() error"); exit(1);}
	else if (readbyte == 0) {
		printf("client disconnected!\n");
		Nyx_client_close(eventfd);
	} else {
		printf("msg_recv: [ %s / %s ]\n", DECODE_STATE(buffer[0]), DECODE_CMD(buffer[1]));	// debug

		cmd_t sent_cmd;

		Nyx_state_sync_check(eventfd, protocol_msg_extract_state(buffer));
		Nyx_state_forward(eventfd, protocol_msg_extract_cmd(buffer));
		sent_cmd = Nyx_state_exec(eventfd, protocol_msg_extract_cmd(buffer), protocol_msg_extract_content(buffer));
		Nyx_state_forward(eventfd, sent_cmd);
		//printf("%s\n", buffer);
	}
}

/* This is dirty, merge this into automata */
void cmd_admin() {
	char buffer_stdin[BUFFER_SIZE];
	scanf("%s", buffer_stdin);

	if (strcmp(buffer_stdin, "shutdown") == 0 || strcmp(buffer_stdin, "sd") == 0) {
		Nyx_server_cleanup();
		Nyx_close();
		exit(0);
	}
}

static void Nyx_readfds_init(fd_set *readfds) {
	FD_ZERO(readfds);
	FD_SET(0, readfds);
	Nyx_fdset_addlistenfd(readfds);
	Nyx_onlineTable_fill_fdset(readfds);
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
		} else if (eventfd == 0) {
			cmd_admin();
		} else {
			task_from_client(eventfd);
		}
	}

	Nyx_close();
	return 0;
}