#include "protocol.h"

#define PACK(signal, state) protocol_state_trans_pack(state, signal)

static short TransTable[ STATELIST_MAX_STATES ][ STATELIST_LENGTH ];



static int protocol_state_trans_pack(const char nextstate, const char trans_signal_cmd) {
	int state = nextstate;
	int statetrans_pack = (state << 8) + trans_signal_cmd;

	//printf("[state/trans_sig_cmd] %d / %d\n", state, (int)trans_signal_cmd);
	//printf("state_trans_pack =    %d\n", statetrans_pack);


	return statetrans_pack;
}

static void protocol_state_TransTable_fill_array(char transTableIndex, int count, ...) {
	va_list args_ptr;

	va_start(args_ptr, count);

	int i;
	for (i = 0; i < count; i++) {
		TransTable[(short)transTableIndex][i] = (short)va_arg(args_ptr, int);
	}

	TransTable[(short)transTableIndex][count] = PACK(CMD_END_OF_CMDLIST, 0);

	va_end(args_ptr);
}

int protocol_state_in_login_session(int state) {
	return STATE_LOGIN_MIN <= state && state < STATE_LOGIN_MAX;
}

int protocol_state_in_console_session(int state) {
	return STATE_CONSOLE_MIN <= state && state < STATE_CONSOLE_MAX;
}

void protocol_state_TransTable_init() {
	protocol_state_TransTable_fill_array(STATE_LOGIN_0, 2, 	PACK(CMD_LOGIN_LOGINPLEASE,		STATE_LOGIN_0),	PACK(CMD_LOGIN_ENTERID,		STATE_LOGIN_1));
	protocol_state_TransTable_fill_array(STATE_LOGIN_1, 2, 	PACK(CMD_LOGIN_MATCH,			STATE_LOGIN_2), PACK(CMD_LOGIN_NOMATCH,		STATE_LOGIN_3));
	protocol_state_TransTable_fill_array(STATE_LOGIN_2, 2, 	PACK(CMD_LOGIN_ENTERPASS,		STATE_LOGIN_4),	PACK(CMD_LOGIN_TYPOID,		STATE_LOGIN_5));
	protocol_state_TransTable_fill_array(STATE_LOGIN_3, 2, 	PACK(CMD_LOGIN_TYPOID,			STATE_LOGIN_5), PACK(CMD_LOGIN_REGISTER,	STATE_LOGIN_6));
	protocol_state_TransTable_fill_array(STATE_LOGIN_4, 2, 	PACK(CMD_LOGIN_WRONGPASS,		STATE_LOGIN_7),	PACK(CMD_LOGIN_LOGINSUCCESS,STATE_CONSOLE));
	protocol_state_TransTable_fill_array(STATE_LOGIN_5, 2, 	PACK(CMD_LOGIN_MATCH,			STATE_LOGIN_2), PACK(CMD_LOGIN_NOMATCH, 	STATE_LOGIN_3));
	protocol_state_TransTable_fill_array(STATE_LOGIN_6, 1, 	PACK(CMD_LOGIN_LOGINSUCCESS,	STATE_CONSOLE));
	protocol_state_TransTable_fill_array(STATE_LOGIN_7, 2, 	PACK(CMD_LOGIN_ENTERPASS,		STATE_LOGIN_4),	PACK(CMD_LOGIN_TYPOID, 		STATE_LOGIN_5));
	protocol_state_TransTable_fill_array(STATE_CONSOLE, 12,	PACK(CMD_CHANGE_STATE_LOGOUT,	STATE_LOGIN_0),
															PACK(CMD_USER_LIST_ALL,			STATE_CONSOLE),
															PACK(CMD_USER_LIST_ONLINE,		STATE_CONSOLE),
															PACK(CMD_USER_RETURN_LIST,		STATE_CONSOLE),
															PACK(CMD_USER_UPDATE_ONLINE,	STATE_CONSOLE),
															PACK(CMD_CHAT_LIST_ALL,			STATE_CONSOLE),
															PACK(CMD_CHAT_RETURN_LIST,		STATE_CONSOLE),
															PACK(CMD_CHAT_FETCH_DIALOG,		STATE_CONSOLE),
															PACK(CMD_CHAT_RETURN_DIALOG,	STATE_CONSOLE),
															PACK(CMD_CHAT_SENDMSG,			STATE_CONSOLE),
															PACK(CMD_CHAT_UPDATE_DIALOG,	STATE_CONSOLE),
															PACK(CMD_FTRANS_REQUEST,		STATE_CONSOLE));
}

void protocol_state_forward(char *state, const char signal_cmd) {
	short i;
	//printf("state = 	 %d\n", *state);
	//printf("signal_cmd = %d\n", signal_cmd);
	printf("forwarding [%s < %s] ---> ", DECODE_STATE(*state), DECODE_CMD(signal_cmd));
	for (i = 0; (char)TransTable[(short)*state][i] != CMD_END_OF_CMDLIST && i < STATELIST_MAX_TRANSITION; i++) {
		//printf("trans_signal_cmd = %d\n", (char)TransTable[(short)*state][i]);
		if ((char)TransTable[(short)*state][i] == signal_cmd) {
			*state = (char)(TransTable[(short)*state][i] >> 8);
			printf("%s\n", DECODE_STATE(*state));
			return;
		}
	}
	//printf("state:		%d\n", *state);
	//printf("sig_cmd: 	%d\n", signal_cmd);

	perror("protocol_state_forward() error:");
	printf("\non %s\nrcv %s\n", DECODE_STATE(*state), DECODE_CMD 	(signal_cmd));
	exit(1);
}

/*
void shift_msg(char *message) {
	short *ptr;
	ptr = (short *)message;
	int i;
	short temp = 0;
	for (i = 1; i < sizeof(message)/2; i++) {
		ptr[i] = temp;
		ptr[i] = ptr[i-1];
	}
}*/

void protocol_msg_send(int fd, char signal_state, char signal_cmd, char *message) {
	char buffer[BUFFER_SIZE] = {signal_state, signal_cmd, 0};		
	if (message != NULL) {
		memmove((buffer + 2), message, sizeof(buffer)-2);
	}
	
	write(fd, buffer, sizeof(buffer));
	printf("msg_send: [ %s / %s ]\n", DECODE_STATE(buffer[0]), DECODE_CMD(buffer[1]));	// debug
}

void protocol_msg_extract() {;}

const char *protocol_decode_state(char state_num) {
	switch(state_num) {
		case STATE_LOGIN_0:
		return "STATE_LOGIN_0";
		case STATE_LOGIN_1:
		return "STATE_LOGIN_1";
		case STATE_LOGIN_2:
		return "STATE_LOGIN_2";
		case STATE_LOGIN_3:
		return "STATE_LOGIN_3";
		case STATE_LOGIN_4:
		return "STATE_LOGIN_4";
		case STATE_LOGIN_5:
		return "STATE_LOGIN_5";
		case STATE_LOGIN_6:
		return "STATE_LOGIN_6";
		case STATE_LOGIN_7:
		return "STATE_LOGIN_7";
		case STATE_CONSOLE:
		return "STATE_CONSOLE";
	}
	printf("failaure when decoding %d\n", (int)state_num);
	exit(1);
}

const char *protocol_decode_cmd(char cmd_num) {
	switch (cmd_num & CMD_MASK_SESSION) {
		case CMD_MASK_LOGIN:
		switch (cmd_num) {
			case CMD_LOGIN_LOGINPLEASE:
			return "CMD_LOGIN_LOGINPLEASE";
			case CMD_LOGIN_ENTERID:
			return "CMD_LOGIN_ENTERID";
			case CMD_LOGIN_MATCH:
			return "CMD_LOGIN_MATCH";
			case CMD_LOGIN_NOMATCH:
			return "CMD_LOGIN_NOMATCH";
			case CMD_LOGIN_ENTERPASS:
			return "CMD_LOGIN_ENTERPASS";
			case CMD_LOGIN_TYPOID:
			return "CMD_LOGIN_TYPOID";
			case CMD_LOGIN_REGISTER:
			return "CMD_LOGIN_REGISTER";
			case CMD_LOGIN_WRONGPASS:
			return "CMD_LOGIN_WRONGPASS";
			case CMD_LOGIN_LOGINSUCCESS:
			return "CMD_LOGIN_LOGINSUCCESS";
		}
		break;
		case CMD_MASK_USER:
		switch (cmd_num) {
			case CMD_USER_LIST_ALL:
			return "CMD_USER_LIST_ALL";
			case CMD_USER_LIST_ONLINE:
			return "CMD_USER_LIST_ONLINE";
			case CMD_USER_RETURN_LIST:
			return "CMD_USER_RETURN_LIST";
			case CMD_USER_UPDATE_ONLINE:
			return "CMD_USER_UPDATE_ONLINE";
		}
		break;
		case CMD_MASK_CHAT:
		switch (cmd_num) {
			case CMD_CHAT_LIST_ALL:
			return "CMD_CHAT_LIST_ALL";
			case CMD_CHAT_RETURN_LIST:
			return "CMD_CHAT_RETURN_LIST";
			case CMD_CHAT_FETCH_DIALOG:
			return "CMD_CHAT_FETCH_DIALOG";
			case CMD_CHAT_RETURN_DIALOG:
			return "CMD_CHAT_RETURN_DIALOG";
			case CMD_CHAT_SENDMSG:
			return "CMD_CHAT_SENDMSG";
			case CMD_CHAT_UPDATE_DIALOG:
			return "CMD_CHAT_UPDATE_DIALOG";
		}
		break;
		case CMD_MASK_FTRANS:
		return "CMD_FTRANS_REQUEST";
		break;
		case CMD_MASK_CHANGE_STATE:
		return "CMD_CHANGE_STATE_LOGOUT";
		break;
	}
	printf("failaure when decoding %d\n", (int)cmd_num);
	exit(1);
}
