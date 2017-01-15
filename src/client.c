#include "Xenia.h"

int main(void) {
	Xenia_init();
	Xenia_connect();

	while (1) {
		if (Xenia_in_login_session()) {
			char buffer[BUFFER_SIZE];
			Xenia_read(buffer, sizeof(buffer));
			char send_signal_cmd;

			Xenia_state_sync_check(buffer[0]);
			Xenia_state_forward(buffer[1]);
			send_signal_cmd = Xenia_state_exec(&buffer[2]);
			Xenia_state_forward(send_signal_cmd);

			//printf("\n\n\n\n");
		}
	}

	Xenia_close();
	return 0;
}