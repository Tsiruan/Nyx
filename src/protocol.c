#include "protocol.h"

static int protocol_state_trans_pack(state_t nextstate, cmd_t trans_signal_cmd);
static void protocol_state_TransTable_fill_array(char transTableIndex, int count, ...);


#define PACK(signal, state) protocol_state_trans_pack(state, signal)

static transition_t TransTable[ STATELIST_MAX_STATES ][ STATELIST_LENGTH ];

/*
 *  The packet sends between client and server has a header:
 *  ------------------------------------------------------------------------
 *  |        state         |          cmd         |       messages.....
 *  ------------------------------------------------------------------------
 *  |<--- 8 bits, char --->|<--- 8 bits, char --->|
 *
 *
 *
 *  The complete information of DFA is stored in the Transition Table, by this format:
 *   
 *   | current |     command     |  next   |
 *   |  state  |  ------------>  |  state  |
 *
 *  TransTable[current state][] = {transition 1, transition 2, ...}
 *  short transition =  (nextState, command)
 *        ^ 2 byte ^     ^ char ^   ^ char ^
 */



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

static int protocol_state_trans_pack(state_t nextstate, cmd_t trans_signal_cmd) {
	int state = nextstate;
	int statetrans_pack = (state << 8) + trans_signal_cmd;

	return statetrans_pack;
}

static void protocol_state_TransTable_fill_array(state_t currentState, int count, ...) {
	va_list args_ptr;
	va_start(args_ptr, count);

	int i;
	for (i = 0; i < count; i++) {
		TransTable[(int)currentState][i] = (transition_t)va_arg(args_ptr, int);
	}

	TransTable[(int)currentState][count] = PACK(CMD_END_OF_CMDLIST, 0);
	va_end(args_ptr);
}

state_t protocol_state_TransTable_lookup(const state_t currentState, const cmd_t rcvcmd) {
	int i;

	for (i = 0; (cmd_t)TransTable[(int)currentState][i] != CMD_END_OF_CMDLIST && i < STATELIST_MAX_TRANSITION; i++) {
		if ((cmd_t)TransTable[(int)currentState][i] == rcvcmd) {
			//printf("%s\n", DECODE_STATE(currentState));
			return (state_t)(TransTable[(int)currentState][i] >> 8);
		}
	}

	perror("transTable_lookup() error");
	printf("\non %s\nrcv %s\n", DECODE_STATE(currentState), DECODE_CMD(rcvcmd));
	exit(1);
}

void protocol_state_forward(state_t *state, const cmd_t signal_cmd) {
	printf("forwarding [%s > %s] ---> ", DECODE_STATE(*state), DECODE_CMD(signal_cmd));

	*state = protocol_state_TransTable_lookup(*state, signal_cmd);

	printf("[%s]\n", DECODE_STATE(*state));
}

/* These two function are quite bad approaches */
int protocol_state_in_login_session(state_t state) {
	return STATE_LOGIN_MIN <= state && state < STATE_LOGIN_MAX;
}

int protocol_state_in_console_session(state_t state) {
	return STATE_CONSOLE_MIN <= state && state < STATE_CONSOLE_MAX;
}


void protocol_msg_send(int fd, state_t signal_state, cmd_t signal_cmd, msg_t message) {
	packet_elem_t buffer[BUFFER_SIZE] = {signal_state, signal_cmd};		
	if (message != NULL) {
		memmove((buffer + 2), message, sizeof(buffer)-2);
	}
	
	write(fd, buffer, sizeof(buffer));
	printf("msg_send: [ %s / %s ]\n", DECODE_STATE(buffer[0]), DECODE_CMD(buffer[1]));	// debug
}

state_t protocol_msg_extract_state(packet_t packet) {
	return packet[0];
}

cmd_t protocol_msg_extract_cmd(packet_t packet) {
	return packet[1];
}

msg_t protocol_msg_extract_content(packet_t packet) {
	return packet + 2;
}

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
