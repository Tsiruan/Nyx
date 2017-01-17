struct OnlineAccount {
	int fd;
	char id[LENGTH_ID_MAX+1];
	//char session;
	state_t state;
	struct sockaddr_in sockaddr;
};

struct OnlineTable {
	int fdcount;
	int fdmax;
	struct OnlineAccount *onlineAccounts;
};

void Nyx_onlineTable_init();
void Nyx_onlineTable_cleanup();

void Nyx_onlineAccount_set(int onlineTable_index, int newfd, char *id, state_t state, struct sockaddr_in *client_in);

void Nyx_onlineTable_push(int newfd, struct sockaddr_in *client_in);
void Nyx_onlineTable_pop(int popfd);
void Nyx_onlineTable_wipe(int wipefd);

void Nyx_onlineTable_userlist_get(char *userlist);
void Nyx_onlineTable_fill_fdset(fd_set *fds);
struct OnlineAccount * Nyx_onlineTable_getbyfd(int fd);
int Nyx_onlineTable_get_maxfd();
int Nyx_onlineTable_find_readfds_isset(fd_set *readfds);
