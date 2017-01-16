#include "Xenia.h"

int main(void) {
	Xenia_init();
	Xenia_connect();

	while (1) {
		if (Xenia_in_login_session()) {
			packet_elem_t buffer[BUFFER_SIZE];
			Xenia_read(buffer, sizeof(buffer));
			cmd_t sent_cmd;

			Xenia_state_sync_check(EXTRACT_STATE(buffer));
			Xenia_state_forward(EXTRACT_CMD(buffer));
			sent_cmd = Xenia_state_exec(EXTRACT_CONTENT(buffer));
			Xenia_state_forward(sent_cmd);

			//printf("\n\n\n\n");
		}
	}

	Xenia_close();
	return 0;
}