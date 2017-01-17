#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 	23519
#define BUFFER_SIZE 	2048
#define LENGTH_ID_MAX 	15
#define LENGTH_PASS_MAX	15
#define LENGTH_MAX 		15	// This is temporary


//#define SESSION_LOGIN		0x00
//#define SESSION_CONSOLE		0x01

#define CMD_END_OF_CMDLIST	0xff

#define CMD_MASK_SESSION		0xf0
#define CMD_MASK_LOGIN			0x00
#define CMD_MASK_USER 			0x10
#define CMD_MASK_CHAT 			0x20
//#define CMD_MASK_MSG 			0x30
#define CMD_MASK_FTRANS			0x40
#define CMD_MASK_CHANGE_STATE 	0x50

#define CMD_LOGIN_LOGINPLEASE	0x01
#define CMD_LOGIN_ENTERID		0X02
#define CMD_LOGIN_MATCH			0x03
#define CMD_LOGIN_NOMATCH		0x04
#define CMD_LOGIN_ENTERPASS		0x05
#define CMD_LOGIN_TYPOID		0x06
#define CMD_LOGIN_REGISTER		0x07
#define CMD_LOGIN_WRONGPASS		0x08
#define CMD_LOGIN_LOGINSUCCESS	0x09

#define CMD_USER_LIST_ALL		0x10
#define CMD_USER_LIST_ONLINE	0x11
#define CMD_USER_RETURN_LIST	0x12
#define CMD_USER_UPDATE_ONLINE	0x13

#define CMD_CHAT_LIST_ALL		0x20
#define CMD_CHAT_RETURN_LIST	0x21
#define CMD_CHAT_FETCH_DIALOG	0x22
#define CMD_CHAT_RETURN_DIALOG	0x23
#define CMD_CHAT_SENDMSG		0x24
#define CMD_CHAT_UPDATE_DIALOG	0x25

//#define CMD_MSG_SEND			0x30

#define CMD_FTRANS_REQUEST			0x40
//#define CMD_FTRANS_RETURN_SOCKADDR	0x41

#define CMD_CHANGE_STATE_LOGOUT		0x50

/* definitions of states has to be continuous, becaues they will become index */
/* This is not a good implementation, fix it afterwards */
#define STATE_LOGIN_MIN	0x01
#define STATE_LOGIN_0	0x01
#define STATE_LOGIN_1	0x02
#define STATE_LOGIN_2	0x03
#define STATE_LOGIN_3	0x04
#define STATE_LOGIN_4	0x05
#define STATE_LOGIN_5	0x06
#define STATE_LOGIN_6	0x07
#define STATE_LOGIN_7	0x08
#define	STATE_LOGIN_MAX	0x09

#define STATE_CONSOLE_MIN	0x09
#define STATE_CONSOLE		0x09
#define STATE_CONSOLE_MAX	0x0A

#define STATELIST_MAX_STATES		10
#define STATELIST_MAX_TRANSITION	16
#define STATELIST_LENGTH STATELIST_MAX_TRANSITION

#define NYX_DATABASE_ACCOUNT_NOMATCH	0
#define NYX_DATABASE_ACCOUNT_MATCH		1
#define NYX_DATABASE_PASS_WRONGPASS		0
#define NYX_DATABASE_PASS_CONFIRM		1


#define DECODE_STATE(state) protocol_decode_state(state)
#define DECODE_CMD(cmd) 	protocol_decode_cmd(cmd)
#define EXTRACT_STATE(msg) 	protocol_msg_extract_state(msg)
#define EXTRACT_CMD(msg) 	protocol_msg_extract_cmd(msg)
#define EXTRACT_CONTENT(msg) protocol_msg_extract_content(msg)

typedef char 			packet_elem_t;
typedef packet_elem_t* 	packet_t;
typedef short 			transition_t;	/* big enough to contain state & cmd */
typedef char 			state_t;
typedef char 			cmd_t;
typedef char*		 	msg_t;


int  protocol_state_in_login_session(state_t state);
int  protocol_state_in_console_session(state_t state);
void protocol_state_TransTable_init();
void protocol_state_forward(state_t *state, const cmd_t signal_cmd);

const char *protocol_decode_state(state_t state_num);
const char *protocol_decode_cmd(cmd_t cmd_num);

void 	protocol_msg_send(int fd, state_t signal_state, cmd_t signal_cmd, msg_t message);
state_t protocol_msg_extract_state(packet_t packet);
cmd_t 	protocol_msg_extract_cmd(packet_t packet);
msg_t 	protocol_msg_extract_content(packet_t packet);

//void protocol_msg_get_sigstate();
//void protocol_msg_get_sigcmd();
