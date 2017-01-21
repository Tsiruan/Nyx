#include "networking.h"
#include "protocol.h"

#define NYX_SELECT_LISTEN -1


/* Nyx.c */
void Nyx_init();
void Nyx_server_cleanup();

void Nyx_listen();
void Nyx_close();
int  Nyx_select(fd_set *readfds);
void Nyx_accept();
void Nyx_client_close(int clientfd);

void Nyx_fdset_push_listenfd(fd_set *fds);
void Nyx_fdset_push_onlineusers(fd_set *fds);

/* Nyx/automata.c */
void  Nyx_state_sync_check(int clientfd, state_t signal_state);
void  Nyx_state_forward(int clientfd, cmd_t signal_cmd);
cmd_t Nyx_state_exec(int clientfd, cmd_t rcvcmd , msg_t message);
