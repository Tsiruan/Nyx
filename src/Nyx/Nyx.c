#include "Nyx.h"
#include "Nyx/onlineTable.h"
#include "Nyx/database.h"


static int Nyxfd;

static cmd_t Nyx_state_exec_session_login(state_t current_state, int clientfd, msg_t message, struct OnlineAccount* onlineAccount);
static cmd_t Nyx_state_exec_session_console(state_t current_state, cmd_t rcvcmd, int clientfd, msg_t message);




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






void Nyx_state_sync_check(int clientfd, state_t signal_state) {
	struct OnlineAccount* onlineAccount = Nyx_onlineTable_getbyfd(clientfd);
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

static cmd_t Nyx_state_exec_session_console_changestate(state_t current_state, cmd_t rcvcmd, int clientfd, msg_t message) {
	switch (rcvcmd) {
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

static cmd_t Nyx_state_exec_session_console_chat(int clientfd, cmd_t rcvcmd, msg_t message) {
	char buffer[BUFFER_SIZE];
	struct OnlineAccount * requestAccount = Nyx_onlineTable_getbyfd(clientfd);

	switch(rcvcmd) {
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

static cmd_t Nyx_state_exec_session_console_user(int clientfd, cmd_t rcvcmd, msg_t message) {
	char userlist[BUFFER_SIZE];

	switch(rcvcmd) {
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

static cmd_t Nyx_state_exec_session_console(state_t current_state, cmd_t rcvcmd, int clientfd, msg_t message) {
	switch(rcvcmd & CMD_MASK_SESSION) {	 
		case CMD_MASK_CHANGE_STATE:
		return Nyx_state_exec_session_console_changestate(current_state, rcvcmd, clientfd, message);
		break;

		case CMD_MASK_USER:
		return Nyx_state_exec_session_console_user(clientfd, rcvcmd, message);
		break;

		case CMD_MASK_CHAT:
		return Nyx_state_exec_session_console_chat(clientfd, rcvcmd, message);
		break;
	}

	perror("Nyx_state_exec_session_console() error");
	return 0;
}

cmd_t Nyx_state_exec(int clientfd, cmd_t rcvcmd, msg_t message) {
	struct OnlineAccount* onlineAccount = Nyx_onlineTable_getbyfd(clientfd);
	state_t exec_state = onlineAccount->state;

	//printf("state num: %d\n", exec_state);
	if (protocol_state_in_login_session(exec_state)) {
		//printf("enter login session\n");
		return Nyx_state_exec_session_login(exec_state, clientfd, message, onlineAccount);
	} else if (protocol_state_in_console_session(exec_state)) {
		//printf("enter console session\n");
		return Nyx_state_exec_session_console(exec_state, rcvcmd, clientfd, message);
	}

	perror("Nyx_state_exec() error");
	exit(1);
}

