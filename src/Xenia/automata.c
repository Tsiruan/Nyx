#include "Xenia.h"
#include "Xenia/automata.h"

extern int 	Nyxfd;
state_t 	Xenia_state;

cmd_t Xenia_state_exec_session_login(msg_t message);
cmd_t Xenia_state_exec_in_console_session(cmd_t cmd, msg_t message);
void Xenia_state_forward_event(cmd_t signal_cmd);


void Xenia_automata_init() {
	Xenia_state = STATE_LOGIN_0;
}

void Xenia_state_sync_check(state_t signal_state) {
	if (signal_state != Xenia_state) {
		printf("Xenia_state: %s\n", DECODE_STATE(Xenia_state));
		printf("sig_state:	 %s\n", DECODE_STATE(signal_state));
		perror("Xenia_state_sync_check() error");
		exit(1);
	}
}

void Xenia_state_forward(cmd_t signal_cmd) {
	Xenia_state_forward_event(signal_cmd);
	protocol_state_forward(&Xenia_state, signal_cmd);
}

cmd_t Xenia_state_exec(cmd_t signal_cmd, msg_t message) {
	if (protocol_state_in_login_session(Xenia_state)) {
		return Xenia_state_exec_session_login(message);
	} else if (protocol_state_in_console_session(Xenia_state)) {
		return Xenia_state_exec_in_console_session(signal_cmd, message);
	}

	perror("Xenia_state_exec() error");
	exit(1);
}








void Xenia_in_state_scanf(char *buffer) {
	scanf("%s", buffer);

	if (ut_strmatch(buffer, 2, "exit", "quit")) {
		Xenia_close();
		exit(0);
	}
}

void Xenia_msg_send(cmd_t signal_cmd, msg_t message) {
	protocol_msg_send(Nyxfd, Xenia_state, signal_cmd, message);
}








void Xenia_state_forward_event(cmd_t signal_cmd) {
	switch (Xenia_state) {
		case STATE_LOGIN_4:
		switch (signal_cmd) {
			case CMD_LOGIN_LOGINSUCCESS:
			printf("You have successfully logged in!\n");
			break;
		}
		break;
		case STATE_LOGIN_6:
		switch (signal_cmd) {
			case CMD_LOGIN_LOGINSUCCESS:
			printf("You have successfully logged in!\n");
			break;
		}
		break;
	}
}


/* return cmd sent out */
cmd_t Xenia_state_exec_session_login(msg_t message) {
	packet_elem_t buffer[LENGTH_MAX];

	switch (Xenia_state) {
		case STATE_LOGIN_0 :
		printf("Welcome!\nEnter your ID: \n");
		Xenia_in_state_scanf(buffer);
		Xenia_msg_send(CMD_LOGIN_ENTERID, buffer);
		return CMD_LOGIN_ENTERID;
		break;


		case STATE_LOGIN_2 :
		printf("Type 'back' to change ID\nPassword: ");
		Xenia_in_state_scanf(buffer);
		if (ut_strmatch(buffer, 1, "back")) {
			printf("Enter your ID: \n");
			Xenia_in_state_scanf(buffer);
			Xenia_msg_send(CMD_LOGIN_TYPOID, buffer);
			return CMD_LOGIN_TYPOID;
		} else {
			Xenia_msg_send(CMD_LOGIN_ENTERPASS, buffer);
			return CMD_LOGIN_ENTERPASS;
		}
		break;


		case STATE_LOGIN_3 :
		printf("No ID matched, want to register? [y/n] \n");
		Xenia_in_state_scanf(buffer);

		while (!ut_strmatch(buffer, 2, "y", "n")) {
			printf("Type 'y' or 'n', want to register? [y/n] ");
			Xenia_in_state_scanf(buffer);
		}
		if (ut_strmatch(buffer, 1, "y")) {
			printf("Enter your password: \n");
			Xenia_in_state_scanf(buffer);
			Xenia_msg_send(CMD_LOGIN_REGISTER, buffer);
			return CMD_LOGIN_REGISTER;

		} else {
			printf("Enter your ID: \n");
			Xenia_in_state_scanf(buffer);
			Xenia_msg_send(CMD_LOGIN_TYPOID, buffer);
			return CMD_LOGIN_TYPOID;
		}
		break;


		case STATE_LOGIN_7 :
		printf("Wrong password! Retry? [y/n] ");
		Xenia_in_state_scanf(buffer);
		while (!ut_strmatch(buffer, 2, "y", "n")) {
			printf("Type 'y' or 'n', want to register? [y/n] ");
			Xenia_in_state_scanf(buffer);
		}
		if (ut_strmatch(buffer, 1, "y")) {
			printf("Enter your password: \n");
			Xenia_in_state_scanf(buffer);
			Xenia_msg_send(CMD_LOGIN_ENTERPASS, buffer);
			return CMD_LOGIN_ENTERPASS;

		} else {
			printf("Enter your ID: \n");
			Xenia_in_state_scanf(buffer);
			Xenia_msg_send(CMD_LOGIN_TYPOID, buffer);
			return CMD_LOGIN_TYPOID;
		}
		break;
	}

	perror("Xenia_state_exec_session_login() error");
	exit(1);
}

void Xenia_exec_chat(char *chat_title) {
	//char buffer[BUFFER_SIZE];

	Xenia_msg_send(CMD_CHAT_FETCH_DIALOG, chat_title);
	// Fetch error shaw be implemented
	/*
	Xenia_read(buffer, sizeof(buffer));
	printf("%s\n", buffer);*/
}





cmd_t Xenia_state_exec_in_console_session(cmd_t cmd, msg_t message) {
	msg_t ptr = message;
	int flag_in_chat = 0;	// take down whenever can


	switch(cmd) {
		case CMD_USER_RETURN_LIST:
		//case CMD_USER_UPDATE_ONLINE:
		case CMD_CHAT_RETURN_LIST:

		while(*ptr != '0') {
			printf("%s\t", ptr);
			ptr += LENGTH_ID_MAX + 1;
		}
		fflush(stdout);
		printf("\n");
		return CMD_NULL;
			break;


		case CMD_CHAT_RETURN_DIALOG:
		case CMD_CHAT_UPDATE_DIALOG:
		printf("%s\n", message);
		return CMD_NULL;
		break;
		

		case CMD_NULL:
		if (ut_strmatch(message, 2, "exit", "quit")) {
			Xenia_close();
			exit(0);
		}
		if (ut_strmatch(message, 1, "logout")) {		// logout
			Xenia_msg_send(CMD_CHANGE_STATE_LOGOUT, NULL);
			printf("Logged out!\n");
			return CMD_CHANGE_STATE_LOGOUT;				// <---- gate out of console state

		} else if (ut_strmatch(message, 1, "l") && !flag_in_chat) {		// list all users
			Xenia_msg_send(CMD_USER_LIST_ALL, NULL);
			return CMD_USER_LIST_ALL;
		} else if (ut_strmatch(message, 1, "lo") == 0 && !flag_in_chat) {		// list all users online
			Xenia_msg_send(CMD_USER_LIST_ONLINE, NULL);
			return CMD_USER_LIST_ONLINE;
		} else if (ut_strmatch(message, 1, "lc") == 0 && !flag_in_chat) {		// list all chats
			Xenia_msg_send(CMD_CHAT_LIST_ALL, NULL);
			return CMD_CHAT_LIST_ALL;
			/*
		} else if (strncmp(message, "#chat", 6) == 0) {		// start a chat
			flag_in_chat = 1;
			Xenia_exec_chat(message + 6);
		} else if (strcmp(message, "#leave") == 0) {
			flag_in_chat = 0;
		} else if (flag_in_chat) {
			Xenia_msg_send(CMD_CHAT_SENDMSG, message);
			*/
		} else {
			printf("unkown command!\n");
		}
		break;
	}
	return 0;
}
