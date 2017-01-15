#include "networking.h"

#define FILE_MAX 4096
#define NAME_MAX 100
#define MSG_MAX 1024
#define PATH_MAX 512
#define INPUT_MAX 4096


struct sendfile_packet
{
    int fd;
    int filesize;
    char path[PATH_MAX];
    char filebuffer[FILE_MAX];
    char name[NAME_MAX];
};

void send_request(int eventfd, int targetfd); //server use to handle send file request
void send_file(int serverfd, char* path,char* name); //client use to send the request to server to send files
void parse(int serverfd, char* input); // client use when get a send command in console
void recv_file(int serverfd); // client use to receive file from other
//char* filename(char* path); //transfer a path to filename

