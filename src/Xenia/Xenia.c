#include "Xenia.h"
#include "Xenia/automata.h"

int 			Nyxfd;
extern state_t 	Xenia_state;


void Xenia_init() {
	network_init_for_windows();
	protocol_state_TransTable_init();
	Xenia_automata_init();
}

void Xenia_connect() {
	Nyxfd = network_connect(SERVER_IP, SERVER_PORT);
}

void Xenia_close() {
	network_close(Nyxfd);
	exit(0);
}

int Xenia_select(fd_set *readfds) {
	if (select(Nyxfd+1, readfds, NULL, NULL, NULL) < 0) {
		fprintf(stderr, "select() error, at %s:%d\n", __FILE__, __LINE__);
		exit(1);
	}

	if (FD_ISSET(Nyxfd, readfds))
		return XENIA_SELECT_SERVER;
	else if (FD_ISSET(0, readfds))
		return 0;

	perror("Xenia_select() error!");
	exit(1);
}

void Xenia_readfds_init(fd_set *readfds) {
	FD_ZERO(readfds);
	FD_SET(Nyxfd, readfds);
	FD_SET(0, readfds);	// stdin
}

void Xenia_read(packet_elem_t *buffer, int size) {
	int readbyte = read(Nyxfd, buffer, size);
	if (readbyte == 0) {			// server closed
		printf("server closed!\n");
		Xenia_close();
		exit(0);
	}

	printf("msg_recv: [ %s / %s ]\n", DECODE_STATE(buffer[0]), DECODE_CMD(buffer[1]));	// debug

}

void Xenia_scanf(packet_t buffer) {
	protocol_msg_scanf(Xenia_state, CMD_NULL, buffer);
}
