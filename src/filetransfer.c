#include "filetransfer.h"

void PathtoFilename(char* path, char* name);

void send_request(int eventfd, int targetfd)
{
    int readbyte;
    struct sendfile_packet* seq = (struct sendfile_packet*)malloc(sizeof(struct sendfile_packet*));
    readbyte = read(eventfd, seq, sizeof(seq));
    if (readbyte< 0) {perror("read() error"); exit(1);}
    seq->fd = targetfd;
    write(eventfd, seq, sizeof(seq));

}

void send_file(int serverfd, char* path, char* name)
{
    char filebuffer[FILE_MAX];
    //printf("%s\n",flag);
    //printf("%s %s\n", name, path);
    FILE* fp = fopen(path, "r");
    int filesize = fread(filebuffer, sizeof(char), FILE_MAX, fp);
    //send message

    //printf("start send\n");
    char msg[5] = "send";
    //printf("*\n");
    write(serverfd, msg, sizeof(msg));
    struct sendfile_packet* seq = (struct sendfile_packet*)malloc(sizeof(struct sendfile_packet*));
    char filename[PATH_MAX];
    PathtoFilename(path, filename);
    strcpy(seq->path, filename);
    strcpy(seq->filebuffer, filebuffer);
    strcpy(seq->name, name);
    seq->filesize = filesize;
    write(serverfd, seq, sizeof(seq));
    //printf("send end\n");
}

void parse(int serverfd, char* input)
{
	char flag1[5], flag2[5], input1[PATH_MAX], input2[PATH_MAX];
	scanf("%s %s %s %s", flag1, input1, flag2, input2);
	//printf("*\n");
	if(!strcmp(flag1, "-u")) send_file(serverfd, input2, input1);
	else send_file(serverfd, input1, input2);
        //printf("%s\n",input);
}

void PathtoFilename(char* path, char* name)
{
    int n = strlen(path);
    int i;
    for(i = n-1; path[i] != '/'; i--);
    int j, x = 0;
    char s[n + 10];
    for(j = i + 1; j < n; j++) s[x++] = path[j];
    return;
}

void recv_file(int serverfd)
{
    struct sendfile_packet* seq = (struct sendfile_packet*)malloc(sizeof(struct sendfile_packet*));
    int readbyte = read(serverfd, seq, sizeof(seq));
    if (readbyte < 0)	{perror("read() error"); exit(1);}
    printf("received a file %s\n", seq->path);
    FILE* fp = fopen(seq->path, "w");
    fwrite(seq->filebuffer, sizeof(char), seq->filesize, fp);
}
