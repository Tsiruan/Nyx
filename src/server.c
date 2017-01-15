#include "Nyx.h"

static void task_from_client(int eventfd) {
	char buffer[BUFFER_SIZE];
	int readbyte;
	readbyte = read(eventfd, buffer, sizeof(buffer));

	if (readbyte < 0)	{perror("read() error"); exit(1);}
	else if (readbyte == 0) {
		printf("client disconnected!\n");
		Nyx_client_close(eventfd);
	} else {
		char send_signal_cmd;
		//printf("\n\n\nmsg_rcv: [state/cmd] %d / %d\n", (int)buffer[0], (int)buffer[1]);	// debug

		Nyx_state_sync_check(eventfd, buffer[0]);
		Nyx_state_forward(eventfd, buffer[1]);
		send_signal_cmd = Nyx_state_exec(eventfd, &buffer[2]);
		Nyx_state_forward(eventfd, send_signal_cmd);
		//printf("%s\n", buffer);
	}
}

void cmd_admin() {
	char buffer[BUFFER_SIZE];
	scanf("%s", buffer);

	if (strcmp(buffer, "shutdown") == 0 || strcmp(buffer, "sd") == 0) {
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