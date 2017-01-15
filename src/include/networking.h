#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WINDOWS_
#include <winsock.h>
#endif

#ifdef _LINUX_
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>
#endif

#define MAX_PENDING_CONNECTION 10

void network_init_for_windows();
int  network_listen(const int server_port);
int  network_connect(const char* server_ip, const int server_port);
int  network_accept(int listenfd, struct sockaddr_in *empty_sockaddr);
void network_close(int srvfd);