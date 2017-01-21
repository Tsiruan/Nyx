#include "Nyx.h"
#include "Nyx/onlineTable.h"
#include "Nyx/database.h"

static int Nyxfd;



void Nyx_init() {
	network_init_for_windows();
	protocol_state_TransTable_init();
	Nyx_onlineTable_init();
	Nyx_database_init();
}

void Nyx_server_cleanup() {
	Nyx_onlineTable_cleanup();
	//free(onlineTable.onlineAccounts);
}

void Nyx_listen() {
	Nyxfd = network_listen(SERVER_PORT);
}

void Nyx_close() {
	network_close(Nyxfd);
}

void Nyx_client_close(int clientfd) {
	Nyx_onlineTable_pop(clientfd);
	network_close(clientfd);
}

int Nyx_select(fd_set *readfds) {
	int max_fd = (Nyx_onlineTable_get_maxfd() > Nyxfd) ? Nyx_onlineTable_get_maxfd() : Nyxfd;
	if (select(max_fd+1, readfds, NULL, NULL, NULL) < 0) {
		fprintf(stderr, "select() error, at %s:%d\n", __FILE__, __LINE__);
		exit(1);
	}

	if (FD_ISSET(Nyxfd, readfds))
		return NYX_SELECT_LISTEN;
	if (FD_ISSET(0, readfds))
		return 0;
	int fd;
	if ((fd = Nyx_onlineTable_find_readfds_isset(readfds)) > 0)
		return fd;

	perror("Nyx_select() error!");
	exit(1);
}

void Nyx_accept() {
	struct sockaddr_in *newclient_in = (struct sockaddr_in *)malloc(sizeof *newclient_in);
	int newclient_fd;

	newclient_fd = network_accept(Nyxfd, newclient_in);

	protocol_msg_send(newclient_fd, STATE_LOGIN_0, CMD_LOGIN_LOGINPLEASE, NULL);
	
	Nyx_onlineTable_push(newclient_fd, newclient_in);
	free(newclient_in);
}

void Nyx_fdset_push_listenfd(fd_set *fds) {
	FD_SET(Nyxfd, fds);
}

void Nyx_fdset_push_onlineusers(fd_set *fds) {
	Nyx_onlineTable_fill_fdset(fds);
}
