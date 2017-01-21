#include "Xenia.h"

int main(void) {
	Xenia_init();
	Xenia_connect();

	fd_set readfds;
	while (1) {
		packet_elem_t buffer[BUFFER_SIZE];
		cmd_t sent_cmd;
		Xenia_readfds_init(&readfds);

		if (Xenia_select(&readfds) == XENIA_SELECT_SERVER) {
			// TODO: fix this to enable client connect to multiple servers
			Xenia_read(buffer, sizeof(buffer));
		} else {
			Xenia_scanf(buffer);
		}

		
		Xenia_state_sync_check(EXTRACT_STATE(buffer));
		Xenia_state_forward(EXTRACT_CMD(buffer));
		sent_cmd = Xenia_state_exec(EXTRACT_CMD(buffer), EXTRACT_CONTENT(buffer));
		Xenia_state_forward(sent_cmd);
	}

	Xenia_close();
	return 0;
}
