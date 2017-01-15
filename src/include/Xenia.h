#include "networking.h"
#include "protocol.h"

#define SERVER_IP "127.0.0.1"

void Xenia_init();
void Xenia_connect();
void Xenia_close();
void Xenia_read(char *buffer, int size);
int  Xenia_in_login_session();
void Xenia_state_sync_check(char signal_state);
void Xenia_state_forward(char signal_cmd);
cmd_t Xenia_state_exec(packet_t message);
