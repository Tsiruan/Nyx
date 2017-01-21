#include "networking.h"
#include "protocol.h"
#include "utility.h"

#define SERVER_IP "127.0.0.1"
#define XENIA_SELECT_SERVER -1

/* Xenia.h */
void Xenia_init();
void Xenia_connect();
void Xenia_close();
void Xenia_read(char *buffer, int size);
void Xenia_readfds_init(fd_set *readfds);
int Xenia_select(fd_set *readfds);
void Xenia_scanf(packet_t buffer);

/* Xenia/automata.h */
void Xenia_state_sync_check(char signal_state);
void Xenia_state_forward(char signal_cmd);
cmd_t Xenia_state_exec(cmd_t cmd, msg_t message);
