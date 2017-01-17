#include "protocol.h"
#include "networking.h"
#include "Nyx/onlineTable.h"

static struct OnlineTable onlineTable;



void Nyx_onlineTable_init() {
	onlineTable.fdcount = 0;
	onlineTable.fdmax = 0;
	onlineTable.onlineAccounts = NULL;
}

void Nyx_onlineTable_cleanup() {
	free(onlineTable.onlineAccounts);
}





void Nyx_onlineAccount_set(int onlineTable_index, int newfd, char *id, state_t state, struct sockaddr_in *client_in) {
	struct OnlineAccount *onlineAccount = &onlineTable.onlineAccounts[onlineTable_index];

	onlineAccount->fd = newfd;
	//onlineAccount->session = session;
	onlineAccount->state = state;
	onlineAccount->sockaddr = *client_in;

	if (id == NULL) {
		memset(onlineAccount->id, 0, sizeof(onlineAccount->id));
	} else {
		strncpy(onlineAccount->id, id, LENGTH_ID_MAX+1);
	}
}





void Nyx_onlineTable_push(int newfd, struct sockaddr_in *client_in) {
	onlineTable.fdcount++;
	if (newfd > onlineTable.fdmax) {
		onlineTable.fdmax = newfd;
	}

	int fdcount = onlineTable.fdcount;
	onlineTable.onlineAccounts = realloc(onlineTable.onlineAccounts, sizeof(*onlineTable.onlineAccounts) * fdcount);
	Nyx_onlineAccount_set(fdcount-1, newfd, NULL,/* SESSION_LOGIN,*/ STATE_LOGIN_0, client_in);
}


void Nyx_onlineTable_pop(int popfd) {
	struct OnlineAccount * popAccount = Nyx_onlineTable_getbyfd(popfd);
	*popAccount = onlineTable.onlineAccounts[onlineTable.fdcount-1];
	onlineTable.fdcount--;
}

void Nyx_onlineTable_wipe(int wipefd) {
	struct OnlineAccount * wipeAccount = Nyx_onlineTable_getbyfd(wipefd);

	wipeAccount->id[0] = '\0';
	wipeAccount->state = STATE_LOGIN_0;
	memset((void *)&wipeAccount->sockaddr, 0, sizeof(wipeAccount->sockaddr));
}





void Nyx_onlineTable_userlist_get(char *userlist) {
	char *ptr = userlist;

	int i;
	for (i = 0; i < onlineTable.fdcount; i++) {
		strcpy(ptr, onlineTable.onlineAccounts[i].id);
		ptr += LENGTH_ID_MAX + 1;
		printf("userlist: %s\n", ptr);
	}
	ptr = 0;
}

void Nyx_onlineTable_fill_fdset(fd_set *fds) {
	int i;
	for (i = 0; i < onlineTable.fdcount; i++) {
		FD_SET(onlineTable.onlineAccounts[i].fd, fds);
	}
}

struct OnlineAccount * Nyx_onlineTable_getbyfd(int fd) {
	int i;
	for (i = 0; i < onlineTable.fdcount; i++) {
		if (onlineTable.onlineAccounts[i].fd == fd) {
			return &onlineTable.onlineAccounts[i];
		}
	}
	perror("Nyx_onlineTable_getbyfd() error");
	exit(1);
}

int Nyx_onlineTable_get_maxfd() {
	return onlineTable.fdmax;
}

int Nyx_onlineTable_find_readfds_isset(fd_set *readfds) {
	int i;
	for (i = 0; i < onlineTable.fdcount; i++) {
			if (FD_ISSET(onlineTable.onlineAccounts[i].fd, readfds))
				return onlineTable.onlineAccounts[i].fd;
	}
	return -1;
}
