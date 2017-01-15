#include <sys/stat.h>
#include <fcntl.h>
#include "networking.h"
#include "protocol.h"

#define NYX_SELECT_LISTEN -1


void Nyx_init();
void Nyx_server_cleanup();

void Nyx_listen();
void Nyx_close();
int  Nyx_select(fd_set *readfds);
void Nyx_accept();
void Nyx_client_close(int clientfd);

void Nyx_fdset_addlistenfd(fd_set *fds);
void Nyx_onlineTable_fill_fdset(fd_set *fds);

void Nyx_state_sync_check(int clientfd, state_t signal_state);
void Nyx_state_forward(int clientfd, cmd_t signal_cmd);
cmd_t Nyx_state_exec(int clientfd, cmd_t rcvcmd , msg_t message);
