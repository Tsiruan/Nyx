#include "Nyx.h"

#define NYX_DATABASE_ACCOUNT_CFG_FILE "./bin/cfg/accounts.cfg"
#define NYX_DATABASE_PATH_CFG_FILES "./bin/cfg/"


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

static int Nyxfd;
static struct OnlineTable onlineTable;
static struct OnlineAccount * Nyx_onlineTable_getbyfd(int fd);
static void Nyx_onlineTable_init();
static void Nyx_onlineAccount_set(int onlineTable_index, int newfd, char *id,/* char session,*/ char state, struct sockaddr_in *client_in);
static void Nyx_onlineTable_push(int newfd, struct sockaddr_in *client_in);
static void Nyx_onlineTable_pop(int popfd);
static char Nyx_state_exec_session_login(char current_state, int clientfd, char *message, struct OnlineAccount* onlineAccount);
static char Nyx_state_exec_session_console(char current_state, int clientfd, char *message);
static void Nyx_database_init();
static int  Nyx_database_account_lookupID(char *id);
static int  Nyx_database_passwork_check(char *id, char *password);
static void Nyx_database_userlist_get(char *userlist);
static void Nyx_database_chat_getbyuser(char *chatlist, char *id);
static void Nyx_database_chat_dialog_get(char *dialog, char *id, char *chat_title);
static void Nyx_database_chat_message_append(char *id, char *chat_title, char *message);
static void Nyx_database_account_new(char *id, char *password);



void Nyx_init() {
	network_init_for_windows();
	protocol_state_TransTable_init();
	Nyx_onlineTable_init();
	Nyx_database_init();
}

void Nyx_server_cleanup() {
	free(onlineTable.onlineAccounts);
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
	int max_fd = (onlineTable.fdmax > Nyxfd) ? onlineTable.fdmax : Nyxfd;
	if (select(max_fd+1, readfds, NULL, NULL, NULL) < 0) {
		fprintf(stderr, "select() error, at %s:%d\n", __FILE__, __LINE__);
		exit(1);
	}

	if (FD_ISSET(Nyxfd, readfds))
		return NYX_SELECT_LISTEN;
	if (FD_ISSET(0, readfds))
		return 0;
	int i;
	for (i = 0; i < onlineTable.fdcount; i++) {
		if (FD_ISSET(onlineTable.onlineAccounts[i].fd, readfds))
			return onlineTable.onlineAccounts[i].fd;
	}
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

void Nyx_fdset_addlistenfd(fd_set *fds) {
	FD_SET(Nyxfd, fds);
}



/* ====================== Nyx_onlineTable ======================= */



static void Nyx_onlineAccount_set(int onlineTable_index, int newfd, char *id, state_t state, struct sockaddr_in *client_in) {
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

static void Nyx_onlineTable_userlist_get(char *userlist) {
	char *ptr = userlist;

	int i;
	for (i = 0; i < onlineTable.fdcount; i++) {
		strcpy(ptr, onlineTable.onlineAccounts[i].id);
		ptr += LENGTH_ID_MAX + 1;
		printf("userlist: %s\n", ptr);
	}
	ptr = 0;
}

static void Nyx_onlineTable_pop(int popfd) {
	struct OnlineAccount * popAccount = Nyx_onlineTable_getbyfd(popfd);
	*popAccount = onlineTable.onlineAccounts[onlineTable.fdcount-1];
	onlineTable.fdcount--;
}

static void Nyx_onlineTable_wipe(int wipefd) {
	struct OnlineAccount * wipeAccount = Nyx_onlineTable_getbyfd(wipefd);

	wipeAccount->id[0] = '\0';
	wipeAccount->state = STATE_LOGIN_0;
	memset((void *)&wipeAccount->sockaddr, 0, sizeof(wipeAccount->sockaddr));
}

static void Nyx_onlineTable_push(int newfd, struct sockaddr_in *client_in) {
	onlineTable.fdcount++;
	if (newfd > onlineTable.fdmax) {
		onlineTable.fdmax = newfd;
	}

	int fdcount = onlineTable.fdcount;
	onlineTable.onlineAccounts = realloc(onlineTable.onlineAccounts, sizeof(*onlineTable.onlineAccounts) * fdcount);
	Nyx_onlineAccount_set(fdcount-1, newfd, NULL,/* SESSION_LOGIN,*/ STATE_LOGIN_0, client_in);

	//printf("New connection!!!\n");
}

static void Nyx_onlineTable_init() {
	onlineTable.fdcount = 0;
	onlineTable.fdmax = 0;
	onlineTable.onlineAccounts = NULL;
}

void Nyx_onlineTable_fill_fdset(fd_set *fds) {
	int i;
	for (i = 0; i < onlineTable.fdcount; i++) {
		FD_SET(onlineTable.onlineAccounts[i].fd, fds);
	}
}

static struct OnlineAccount * Nyx_onlineTable_getbyfd(int fd) {
	int i;
	for (i = 0; i < onlineTable.fdcount; i++) {
		if (onlineTable.onlineAccounts[i].fd == fd) {
			return &onlineTable.onlineAccounts[i];
		}
	}
	perror("Nyx_onlineTable_getbyfd() error");
	exit(1);
}



/* ====================== Nyx_state ======================= */




void Nyx_state_sync_check(int clientfd, state_t signal_state) {
	const struct OnlineAccount* onlineAccount = Nyx_onlineTable_getbyfd(clientfd);
	if (onlineAccount->state != signal_state) {
		printf("Nyx_state_sync_check() error \nfd = %d\n", clientfd);
		exit(1);
	}
}

void Nyx_state_forward(int clientfd, cmd_t signal_cmd) {
	struct OnlineAccount* onlineAccount = Nyx_onlineTable_getbyfd(clientfd);
	protocol_state_forward(&onlineAccount->state, signal_cmd);
}

static cmd_t Nyx_state_exec_session_login(state_t current_state, int clientfd, msg_t message, struct OnlineAccount* onlineAccount) {
	switch (current_state) {

		case STATE_LOGIN_0:
		protocol_msg_send(clientfd, current_state, CMD_LOGIN_LOGINPLEASE, NULL);
		return CMD_LOGIN_LOGINPLEASE;

		case STATE_LOGIN_1:
		case STATE_LOGIN_5:
		strncpy(onlineAccount->id, message, LENGTH_ID_MAX+1);
		printf("%s\n", message);
		
		if (NYX_DATABASE_ACCOUNT_NOMATCH == Nyx_database_account_lookupID(message)) {
			protocol_msg_send(clientfd, current_state, CMD_LOGIN_NOMATCH, NULL);
			return CMD_LOGIN_NOMATCH;
		} else {
			protocol_msg_send(clientfd, current_state, CMD_LOGIN_MATCH, NULL);
			return CMD_LOGIN_MATCH;
		}
		break;


		case STATE_LOGIN_4:
		if (NYX_DATABASE_PASS_WRONGPASS == Nyx_database_passwork_check(onlineAccount->id, message)) {
			protocol_msg_send(clientfd, current_state, CMD_LOGIN_WRONGPASS, NULL);
			return CMD_LOGIN_WRONGPASS;
		} else {
			protocol_msg_send(clientfd, current_state, CMD_LOGIN_LOGINSUCCESS, NULL);
			return CMD_LOGIN_LOGINSUCCESS;
		}
		break;

		
		case STATE_LOGIN_6:
		Nyx_database_account_new(onlineAccount->id, message);

		protocol_msg_send(clientfd, current_state, CMD_LOGIN_LOGINSUCCESS, NULL);
		
		return CMD_LOGIN_LOGINSUCCESS;
		break;
	}

	perror("Nyx_state_exec_session_login() error");
	exit(1);
}

static cmd_t Nyx_state_exec_session_console_changestate(state_t current_state, int clientfd, msg_t message) {
	cmd_t command = *(message -1);
	switch (command) {
		case CMD_CHANGE_STATE_LOGOUT:
		printf("client logged out!\n");
		Nyx_onlineTable_wipe(clientfd);
		protocol_msg_send(clientfd, STATE_LOGIN_0, CMD_LOGIN_LOGINPLEASE, NULL);
		return CMD_LOGIN_LOGINPLEASE;
		break;
	}

	printf("Error in file %s:%d\n", __FILE__, __LINE__);
	exit(1);
}

static cmd_t Nyx_state_exec_session_console_chat(int clientfd, msg_t message) {
	char buffer[BUFFER_SIZE];
	struct OnlineAccount * requestAccount = Nyx_onlineTable_getbyfd(clientfd);

	switch(*(message - 1)) {
		case CMD_CHAT_LIST_ALL:
		Nyx_database_chat_getbyuser(buffer, requestAccount->id);
		protocol_msg_send(clientfd, STATE_LOGIN_0, CMD_CHAT_RETURN_LIST, buffer);
		return CMD_CHAT_RETURN_LIST;
		break;

		case CMD_CHAT_FETCH_DIALOG:
		Nyx_database_chat_dialog_get(buffer, requestAccount->id, message/* chat title */);
		protocol_msg_send(clientfd, STATE_LOGIN_0, CMD_CHAT_RETURN_DIALOG, buffer);
		return CMD_CHAT_RETURN_DIALOG;
		break;

		case CMD_CHAT_SENDMSG:
		Nyx_database_chat_message_append(requestAccount->id, message, &message[16]);
		// TODO send msg to online peers
		return CMD_CHAT_UPDATE_DIALOG;
		break;
	}
	perror("Nyx_state_exec_session_console_chat() error");
	exit(1);
}

static cmd_t Nyx_state_exec_session_console_user(int clientfd, msg_t message) {
	char userlist[BUFFER_SIZE];

	switch(*(message - 1)) {
		case CMD_USER_LIST_ALL:
		Nyx_database_userlist_get(userlist);
		protocol_msg_send(clientfd, STATE_CONSOLE, CMD_USER_RETURN_LIST, userlist);
		return CMD_USER_RETURN_LIST;
		break;


		case CMD_USER_LIST_ONLINE:
		Nyx_onlineTable_userlist_get(userlist);
		protocol_msg_send(clientfd, STATE_CONSOLE, CMD_USER_RETURN_LIST, userlist);
		return CMD_USER_RETURN_LIST;
		break;
	}
	perror("Nyx_state_exec_session_console_user() error");
	exit(1);
}

static cmd_t Nyx_state_exec_session_console(state_t current_state, int clientfd, msg_t message) {
	cmd_t command = *(message - 1);
	switch(command & CMD_MASK_SESSION) {	 
		case CMD_MASK_CHANGE_STATE:
		return Nyx_state_exec_session_console_changestate(current_state, clientfd, message);
		break;

		case CMD_MASK_USER:
		return Nyx_state_exec_session_console_user(clientfd, message);
		break;

		case CMD_MASK_CHAT:
		return Nyx_state_exec_session_console_chat(clientfd, message);
		break;
	}

	perror("Nyx_state_exec_session_console() error");
	return 0;
}

cmd_t Nyx_state_exec(int clientfd, msg_t message) {
	struct OnlineAccount* onlineAccount = Nyx_onlineTable_getbyfd(clientfd);
	state_t current_state = onlineAccount->state;

	//printf("state num: %d\n", current_state);
	if (protocol_state_in_login_session(current_state)) {
		//printf("enter login session\n");
		return Nyx_state_exec_session_login(current_state, clientfd, message, onlineAccount);
	} else if (protocol_state_in_console_session(current_state)) {
		//printf("enter console session\n");
		return Nyx_state_exec_session_console(current_state, clientfd, message);
	}

	perror("Nyx_state_exec() error");
	exit(1);
}



/* ====================== Nyx_database ======================= */




static void Nyx_database_init() {
	//system("pwd");

	/* create accounts.cfg if non exist */
	FILE *fp = fopen(NYX_DATABASE_ACCOUNT_CFG_FILE, "a");
	if (fp == NULL) {perror("fopen() fail\n"); exit(1);}
	fclose(fp);
}

static void Nyx_database_userlist_get(char *userlist) {
	char buffer[LENGTH_ID_MAX + LENGTH_PASS_MAX + 2];
	char *ptr = userlist;

	FILE *account_cfg_fp = fopen(NYX_DATABASE_ACCOUNT_CFG_FILE, "r");
	if (account_cfg_fp == NULL) {
		perror("fopen() fail");
		exit(1);
	}

	while (!feof(account_cfg_fp)) {
		fread(buffer, sizeof(char), LENGTH_ID_MAX + LENGTH_PASS_MAX + 2, account_cfg_fp);
		strncpy(ptr, buffer, LENGTH_ID_MAX+1);
		ptr += LENGTH_ID_MAX + LENGTH_PASS_MAX + 2;
	}
	*ptr = '0';

	fclose(account_cfg_fp);
}

static void Nyx_database_chat_getbyuser(char *chatlist, char *id) {
	char cfg_path[50] = {};
	strcat(cfg_path, NYX_DATABASE_PATH_CFG_FILES);
	strcat(cfg_path, id);
	strcat(cfg_path, ".cfg");

	char *ptr = chatlist;
	char buffer[32];
	FILE *user_cfg_fp = fopen(cfg_path, "r");
	while (!feof(user_cfg_fp)) {
		fread(buffer, sizeof(char), sizeof(buffer), user_cfg_fp);
		strncpy(ptr, buffer, 16);
		ptr += 16;
	}

	fclose(user_cfg_fp);
}

static void Nyx_database_chat_dialog_get(char *dialog, char *id, char *chat_title) {
	char cfg_path[50] = {};
	strcat(cfg_path, NYX_DATABASE_PATH_CFG_FILES);
	strcat(cfg_path, id);
	strcat(cfg_path, ".cfg");

	char buffer[32];
	char *chat_file_name;
	FILE *user_cfg_fp = fopen(cfg_path, "r");
	while(!feof(user_cfg_fp)) {
		fread(buffer, sizeof(char), sizeof(buffer), user_cfg_fp);
		if (strncmp(chat_title, buffer, 16) == 0) {
			chat_file_name = buffer + 16;
			break;
		}
	}
	fclose(user_cfg_fp);

	char chat_path[50] = {};
	strcat(chat_path, NYX_DATABASE_PATH_CFG_FILES);
	strcat(chat_path, chat_file_name);
	strcat(chat_path, ".chat");

	FILE *user_chat = fopen(chat_path, "r");
	if (user_chat == NULL)	{perror("fopen() error"); exit(1);}
	fread(dialog, sizeof(char), sizeof(dialog), user_chat);

	fclose(user_chat);
}

static void Nyx_database_chat_message_append(char *id, char *chat_title, char *message) {
	char cfg_path[50] = {};
	strcat(cfg_path, NYX_DATABASE_PATH_CFG_FILES);
	strcat(cfg_path, id);
	strcat(cfg_path, ".cfg");

	char buffer[32];
	char *chat_file_name;
	FILE *user_cfg_fp = fopen(cfg_path, "r");
	while(!feof(user_cfg_fp)) {
		fread(buffer, sizeof(char), sizeof(buffer), user_cfg_fp);
		if (strncmp(chat_title, buffer, 16) == 0) {
			chat_file_name = buffer + 16;
			break;
		}
	}
	fclose(user_cfg_fp);

	char chat_path[50] = {};
	strcat(chat_path, NYX_DATABASE_PATH_CFG_FILES);
	strcat(chat_path, chat_file_name);
	strcat(chat_path, ".chat");

	FILE *user_chat = fopen(chat_path, "a");
	fwrite(message, sizeof(char), sizeof(message), user_chat);
	fclose(user_chat);
}


static int Nyx_database_account_lookupID(char *id) {
	char buffer[LENGTH_ID_MAX + LENGTH_PASS_MAX + 2];

	FILE *account_cfg_fp = fopen(NYX_DATABASE_ACCOUNT_CFG_FILE, "r");
	if (account_cfg_fp == NULL) {
		perror("fopen() fail");
		exit(1);
	}

	while (!feof(account_cfg_fp)) {
		fread(buffer, sizeof(char), LENGTH_ID_MAX + LENGTH_PASS_MAX + 2, account_cfg_fp);
		if (strncmp(id, buffer, LENGTH_ID_MAX + 1) == 0) {
			return NYX_DATABASE_ACCOUNT_MATCH;
		}
	}

	fclose(account_cfg_fp);
	return NYX_DATABASE_ACCOUNT_NOMATCH;
}

static int Nyx_database_passwork_check(char *id, char *password) {
	char buffer[LENGTH_ID_MAX + LENGTH_PASS_MAX + 2];

	FILE *account_cfg_fp = fopen(NYX_DATABASE_ACCOUNT_CFG_FILE, "r");

	while (!feof(account_cfg_fp)) {
		fread(buffer, sizeof(char), LENGTH_ID_MAX + LENGTH_PASS_MAX + 2, account_cfg_fp);
		if (strncmp(password, &buffer[LENGTH_ID_MAX + 1], LENGTH_ID_MAX + 1) == 0) {
			return NYX_DATABASE_PASS_CONFIRM;
		}
	}

	fclose(account_cfg_fp);
	return NYX_DATABASE_PASS_WRONGPASS;
}

static void Nyx_database_account_new(char *id, char *password) {
	char buffer[LENGTH_ID_MAX + LENGTH_PASS_MAX + 2];
	strncpy(buffer, id, LENGTH_ID_MAX + 1);
	strncpy(&buffer[LENGTH_ID_MAX + 1], password, LENGTH_PASS_MAX);

	FILE *account_cfg_fp = fopen(NYX_DATABASE_ACCOUNT_CFG_FILE, "a");
	fwrite(buffer, sizeof(char), LENGTH_ID_MAX + LENGTH_PASS_MAX + 2, account_cfg_fp);
	fclose(account_cfg_fp);

	
	char cfg_path[50] = {};
	strcat(cfg_path, NYX_DATABASE_PATH_CFG_FILES);
	strcat(cfg_path, id);
	strcat(cfg_path, ".cfg");
	int fd = creat(cfg_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		perror("creat() fail\n");
		printf("errno: %d\n", errno);
		exit(1);
	}
	close(fd);
}
