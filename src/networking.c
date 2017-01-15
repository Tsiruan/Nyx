#include "networking.h"



void network_init_for_windows() {
#ifdef _WINDOWS_
	WSADATA wsaData;
	WSAStartup(0x0202, &wsaData);
#endif
}

static void initsocket(struct sockaddr_in *srv, const char* dst_ip, const int port) {
	srv->sin_family = AF_INET;
	srv->sin_port = htons(port);

	if (dst_ip == NULL) {
		srv->sin_addr.s_addr = htonl(INADDR_ANY);
	} else {
		srv->sin_addr.s_addr = inet_addr(dst_ip);
		if (srv->sin_addr.s_addr == -1) {
			perror("inet_addr() fail");
		}
	}

	memset(&(srv->sin_zero), '\0', 8);
}

int network_listen(const int server_port) {
	int srvfd;
	struct sockaddr_in srv;

	initsocket(&srv, NULL, server_port);
	if((srvfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)			{perror("socket() fail");	exit(1);}

	int yes = 1;
	if (setsockopt(srvfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
	    perror("setsockopt");
	    exit(1);
	}

	if(bind(srvfd, (struct sockaddr*) &srv, sizeof(srv)) < 0)	{perror("bind() fail");		exit(1);}
	if(listen(srvfd, MAX_PENDING_CONNECTION) < 0)									{perror("listen() fail");	exit(1);}

	return srvfd;
}

int network_connect(const char* server_ip, const int server_port) {
	int srvfd;
	struct sockaddr_in srv;

	initsocket(&srv, server_ip, server_port);

	if ((srvfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)					{perror("socket() fail"); exit(1);}
	if (connect(srvfd, (struct sockaddr *)&srv, sizeof(srv)) < 0)		{perror("connect() fail"); exit(1);}

	return srvfd;
}

int network_accept(int listenfd, struct sockaddr_in *empty_sockaddr) {
	socklen_t sockin_len = sizeof(struct sockaddr_in);
	int newfd;

	newfd = accept(listenfd, (struct sockaddr *)empty_sockaddr, &sockin_len);
	if (newfd < 0)		{perror("accept() error"); exit(1);}

	return newfd;
}

void network_close(int srvfd) {
#ifdef _LINUX_
	close(srvfd);
#endif
#ifdef _WINDOWS_
	closesocket(srvfd);
	WSACleanup();
#endif
}
