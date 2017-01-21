#include "Nyx.h"
#include "Nyx/onlineTable.h"
#include "Nyx/database.h"

static cmd_t Nyx_state_exec_session_login(state_t current_state, int clientfd, msg_t message, struct OnlineAccount* onlineAccount);
static cmd_t Nyx_state_exec_session_console(state_t current_state, cmd_t rcvcmd, int clientfd, msg_t message);



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

cmd_t Nyx_state_exec(int clientfd, cmd_t rcvcmd, msg_t message) {
	struct OnlineAccount* onlineAccount = Nyx_onlineTable_getbyfd(clientfd);
	state_t exec_state = onlineAccount->state;

	if (protocol_state_in_login_session(exec_state)) {
		return Nyx_state_exec_session_login(exec_state, clientfd, message, onlineAccount);
	} else if (protocol_state_in_console_session(exec_state)) {
		return Nyx_state_exec_session_console(exec_state, rcvcmd, clientfd, message);
	}

	perror("Nyx_state_exec() error");
	exit(1);
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