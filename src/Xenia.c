#include "Xenia.h"

static int 	Nyxfd;
static char Xenia_state;

static void Xenia_stdin_scanf(char *buffer);
static void Xenia_readfds_init(fd_set *readfds);
static char Xenia_state_exec_session_login(char *message);
static char Xenia_state_exec_in_console_session(char *message);
static void Xenia_msg_send(char signal_cmd, char *message);
static void Xenia_exec_chat(char *chat_title);

void Xenia_init();
void Xenia_connect();
void Xenia_close();
void Xenia_read(char *buffer, int size);
int  Xenia_in_login_session();						// Never used outside by now
void Xenia_state_sync_check(char signal_state);
void Xenia_state_forward(char signal_cmd);
char Xenia_state_exec(char *message);



void Xenia_init() {
	network_init_for_windows();
	protocol_state_TransTable_init();
	Xenia_state = STATE_LOGIN_0;
}

void Xenia_connect() {
	Nyxfd = network_connect(SERVER_IP, SERVER_PORT);
}

static int Xenia_select(fd_set *readfds) {
	if (select(Nyxfd+1, readfds, NULL, NULL, NULL) < 0) {
		fprintf(stderr, "select() error, at %s:%d\n", __FILE__, __LINE__);
		exit(1);
	}

	if (FD_ISSET(Nyxfd, readfds))
		return Nyxfd;
	else if (FD_ISSET(0, readfds))
		return 0;

	perror("Xenia_select() error!");
	exit(1);
}

void Xenia_close() {
	network_close(Nyxfd);
	exit(0);
}

void Xenia_read(char *buffer, int size) {
	/*int readbyte;*/
	int readbyte = read(Nyxfd, buffer, size);
	//printf("msg_rcv: [state/cmd] %d / %d\n", (int)buffer[0], (int)buffer[1]);	// debug
	//printf("read byte: %d\n", readbyte);
	if (readbyte == 0) {	// server closed
		printf("server closed!\n");
		Xenia_close();
	}
}

int Xenia_in_login_session() {
	return protocol_state_in_login_session(Xenia_state);
}

static void Xenia_stdin_scanf(char *buffer) {
	scanf("%s", buffer);

	if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "quit") == 0) {
		Xenia_close();
		exit(0);
	}
}

static void Xenia_readfds_init(fd_set *readfds) {
	FD_ZERO(readfds);
	FD_SET(Nyxfd, readfds);
	FD_SET(0, readfds);	// stdin
}



/* ====================== Xenia_state ======================= */




void Xenia_state_sync_check(char signal_state) {
	if (signal_state != Xenia_state) {
		printf("Xenia:		%d\n", (int)Xenia_state);
		printf("sig_state:	%d\n", (int)signal_state);
		perror("Xenia_state_sync_check() error");
		exit(1);
	}
}

void Xenia_state_forward(char signal_cmd) {
	protocol_state_forward(&Xenia_state, signal_cmd);
}

char Xenia_state_exec(char *message) {
	if (protocol_state_in_login_session(Xenia_state)) {
		return Xenia_state_exec_session_login(message);
	} else if (protocol_state_in_console_session(Xenia_state)) {
		return Xenia_state_exec_in_console_session(message);
	}

	perror("Xenia_state_exec() error");
	exit(1);
}

static void Xenia_msg_send(char signal_cmd, char *message) {
	protocol_msg_send(Nyxfd, Xenia_state, signal_cmd, message);
}

/* return cmd sent out */
static char Xenia_state_exec_session_login(char *message) {
	char buffer[LENGTH_MAX];

	switch (Xenia_state) {
		case STATE_LOGIN_0 :
		printf("Welcome!\nEnter your ID: \n");
		Xenia_stdin_scanf(buffer);
		Xenia_msg_send(CMD_LOGIN_ENTERID, buffer);
		return CMD_LOGIN_ENTERID;
		break;


		case STATE_LOGIN_2 :
		printf("Type 'back' to change ID\nPassword: ");
		Xenia_stdin_scanf(buffer);
		if (strcmp(buffer, "back") == 0) {
			printf("Enter your ID: \n");
			Xenia_stdin_scanf(buffer);
			Xenia_msg_send(CMD_LOGIN_TYPOID, buffer);
			return CMD_LOGIN_TYPOID;
		} else {
			Xenia_msg_send(CMD_LOGIN_ENTERPASS, buffer);
			return CMD_LOGIN_ENTERPASS;
		}
		break;


		case STATE_LOGIN_3 :
		printf("No ID matched, want to register? [y/n] \n");
		Xenia_stdin_scanf(buffer);

		while (strcmp(buffer, "y") != 0 && strcmp(buffer, "n") != 0) {
			printf("Type 'y' or 'n', want to register? [y/n] ");
			Xenia_stdin_scanf(buffer);
		}
		if (strcmp(buffer ,"y") == 0) {
			printf("Enter your password: \n");
			Xenia_stdin_scanf(buffer);
			Xenia_msg_send(CMD_LOGIN_REGISTER, buffer);
			return CMD_LOGIN_REGISTER;

		} else {
			printf("Enter your ID: \n");
			Xenia_stdin_scanf(buffer);
			Xenia_msg_send(CMD_LOGIN_TYPOID, buffer);
			return CMD_LOGIN_TYPOID;
		}
		break;

		case STATE_LOGIN_7 :
		printf("Wrong password! Retry? [y/n] ");
		Xenia_stdin_scanf(buffer);
		while (strcmp(buffer, "y") != 0 && strcmp(buffer, "n") != 0) {
			printf("Type 'y' or 'n', want to register? [y/n] ");
			Xenia_stdin_scanf(buffer);
		}
		if (strcmp(buffer, "y") == 0) {
			printf("Enter your password: \n");
			Xenia_stdin_scanf(buffer);
			Xenia_msg_send(CMD_LOGIN_ENTERPASS, buffer);
			return CMD_LOGIN_ENTERPASS;

		} else {
			printf("Enter your ID: \n");
			Xenia_stdin_scanf(buffer);
			Xenia_msg_send(CMD_LOGIN_TYPOID, buffer);
			return CMD_LOGIN_TYPOID;
		}
		break;
	}

	perror("Xenia_state_exec_session_login() error");
	exit(1);
}

static void Xenia_exec_chat(char *chat_title) {
	//char buffer[BUFFER_SIZE];

	Xenia_msg_send(CMD_CHAT_FETCH_DIALOG, chat_title);
	// Fetch error shaw be implemented
	/*
	Xenia_read(buffer, sizeof(buffer));
	printf("%s\n", buffer);*/
}

static char Xenia_state_exec_in_console_session(char *message) {
	char buffer[BUFFER_SIZE];
	printf("You have successfully logged in!\n");

	fd_set readfds;
	while (1) {
		Xenia_readfds_init(&readfds);

		if (Xenia_select(&readfds) == Nyxfd) {
			Xenia_read(buffer, sizeof(buffer));
			char cmd = buffer[1];
			char *ptr = buffer + 2;

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
				break;



				case CMD_CHAT_RETURN_DIALOG:
				case CMD_CHAT_UPDATE_DIALOG:
				printf("%s\n", buffer+2);
				break;
			}

		} else {
											// msg from user
			int flag_in_chat = 0;
			Xenia_stdin_scanf(buffer);
			if (strcmp(buffer, "logout") == 0) {		// logout
				Xenia_msg_send(CMD_CHANGE_STATE_LOGOUT, NULL);
				printf("Logged out!\n");
				return CMD_CHANGE_STATE_LOGOUT;				// <---- gate out of console state

			} else if (strcmp(buffer, "l") == 0 && !flag_in_chat) {		// list all users
				Xenia_msg_send(CMD_USER_LIST_ALL, NULL);
			} else if (strcmp(buffer, "lo") == 0 && !flag_in_chat) {		// list all users online
				Xenia_msg_send(CMD_USER_LIST_ONLINE, NULL);
			} else if (strcmp(buffer, "lc") == 0 && !flag_in_chat) {		// list all chats
				Xenia_msg_send(CMD_CHAT_LIST_ALL, NULL);
			} else if (strncmp(buffer, "#chat", 6) == 0) {		// start a chat
				flag_in_chat = 1;
				Xenia_exec_chat(buffer + 6);
			} else if (strcmp(buffer, "#leave") == 0) {
				flag_in_chat = 0;
			} else {

				if (flag_in_chat) {
					Xenia_msg_send(CMD_CHAT_SENDMSG, buffer);
				} else {
					printf("unkown command!\n");
				}
			}
		}
	}
	return 0;
}
