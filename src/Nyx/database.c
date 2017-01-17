#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "protocol.h"
#include "Nyx/database.h"


void Nyx_database_init() {
	//system("pwd");

	/* create accounts.cfg if non exist */
	FILE *fp = fopen(NYX_DATABASE_ACCOUNT_CFG_FILE, "a");
	if (fp == NULL) {perror("fopen() fail\n"); exit(1);}
	fclose(fp);
}

void Nyx_database_userlist_get(char *userlist) {
	char buffer[LENGTH_ID_MAX + LENGTH_PASS_MAX + 2];
	char *ptr = userlist;

	FILE *account_cfg_fp = fopen(NYX_DATABASE_ACCOUNT_CFG_FILE, "r");
	if (account_cfg_fp == NULL) {
		perror("fopen() fail");
		exit(1);
	}

	while (!feof(account_cfg_fp)) {
		fread(buffer, sizeof(char), LENGTH_ID_MAX + LENGTH_PASS_MAX + 2, account_cfg_fp);
		strncpy(ptr, buffer, LENGTH_ID_MAX+1);
		ptr += LENGTH_ID_MAX + LENGTH_PASS_MAX + 2;
	}
	*ptr = '0';

	fclose(account_cfg_fp);
}

void Nyx_database_chat_getbyuser(char *chatlist, char *id) {
	char cfg_path[50] = {};
	strcat(cfg_path, NYX_DATABASE_PATH_CFG_FILES);
	strcat(cfg_path, id);
	strcat(cfg_path, ".cfg");

	char *ptr = chatlist;
	char buffer[32];
	FILE *user_cfg_fp = fopen(cfg_path, "r");
	while (!feof(user_cfg_fp)) {
		fread(buffer, sizeof(char), sizeof(buffer), user_cfg_fp);
		strncpy(ptr, buffer, 16);
		ptr += 16;
	}

	fclose(user_cfg_fp);
}

void Nyx_database_chat_dialog_get(char *dialog, char *id, char *chat_title) {
	char cfg_path[50] = {};
	strcat(cfg_path, NYX_DATABASE_PATH_CFG_FILES);
	strcat(cfg_path, id);
	strcat(cfg_path, ".cfg");

	char buffer[32];
	char *chat_file_name;
	FILE *user_cfg_fp = fopen(cfg_path, "r");
	while(!feof(user_cfg_fp)) {
		fread(buffer, sizeof(char), sizeof(buffer), user_cfg_fp);
		if (strncmp(chat_title, buffer, 16) == 0) {
			chat_file_name = buffer + 16;
			break;
		}
	}
	fclose(user_cfg_fp);

	char chat_path[50] = {};
	strcat(chat_path, NYX_DATABASE_PATH_CFG_FILES);
	strcat(chat_path, chat_file_name);
	strcat(chat_path, ".chat");

	FILE *user_chat = fopen(chat_path, "r");
	if (user_chat == NULL)	{perror("fopen() error"); exit(1);}
	fread(dialog, sizeof(char), sizeof(dialog), user_chat);

	fclose(user_chat);
}

void Nyx_database_chat_message_append(char *id, char *chat_title, char *message) {
	char cfg_path[50] = {};
	strcat(cfg_path, NYX_DATABASE_PATH_CFG_FILES);
	strcat(cfg_path, id);
	strcat(cfg_path, ".cfg");

	char buffer[32];
	char *chat_file_name;
	FILE *user_cfg_fp = fopen(cfg_path, "r");
	while(!feof(user_cfg_fp)) {
		fread(buffer, sizeof(char), sizeof(buffer), user_cfg_fp);
		if (strncmp(chat_title, buffer, 16) == 0) {
			chat_file_name = buffer + 16;
			break;
		}
	}
	fclose(user_cfg_fp);

	char chat_path[50] = {};
	strcat(chat_path, NYX_DATABASE_PATH_CFG_FILES);
	strcat(chat_path, chat_file_name);
	strcat(chat_path, ".chat");

	FILE *user_chat = fopen(chat_path, "a");
	fwrite(message, sizeof(char), sizeof(message), user_chat);
	fclose(user_chat);
}


int Nyx_database_account_lookupID(char *id) {
	char buffer[LENGTH_ID_MAX + LENGTH_PASS_MAX + 2];

	FILE *account_cfg_fp = fopen(NYX_DATABASE_ACCOUNT_CFG_FILE, "r");
	if (account_cfg_fp == NULL) {
		perror("fopen() fail");
		exit(1);
	}

	while (!feof(account_cfg_fp)) {
		fread(buffer, sizeof(char), LENGTH_ID_MAX + LENGTH_PASS_MAX + 2, account_cfg_fp);
		if (strncmp(id, buffer, LENGTH_ID_MAX + 1) == 0) {
			return NYX_DATABASE_ACCOUNT_MATCH;
		}
	}

	fclose(account_cfg_fp);
	return NYX_DATABASE_ACCOUNT_NOMATCH;
}

int Nyx_database_passwork_check(char *id, char *password) {
	char buffer[LENGTH_ID_MAX + LENGTH_PASS_MAX + 2];

	FILE *account_cfg_fp = fopen(NYX_DATABASE_ACCOUNT_CFG_FILE, "r");

	while (!feof(account_cfg_fp)) {
		fread(buffer, sizeof(char), LENGTH_ID_MAX + LENGTH_PASS_MAX + 2, account_cfg_fp);
		if (strncmp(password, &buffer[LENGTH_ID_MAX + 1], LENGTH_ID_MAX + 1) == 0) {
			return NYX_DATABASE_PASS_CONFIRM;
		}
	}

	fclose(account_cfg_fp);
	return NYX_DATABASE_PASS_WRONGPASS;
}

void Nyx_database_account_new(char *id, char *password) {
	char buffer[LENGTH_ID_MAX + LENGTH_PASS_MAX + 2];
	strncpy(buffer, id, LENGTH_ID_MAX + 1);
	strncpy(&buffer[LENGTH_ID_MAX + 1], password, LENGTH_PASS_MAX);

	FILE *account_cfg_fp = fopen(NYX_DATABASE_ACCOUNT_CFG_FILE, "a");
	fwrite(buffer, sizeof(char), LENGTH_ID_MAX + LENGTH_PASS_MAX + 2, account_cfg_fp);
	fclose(account_cfg_fp);

	
	char cfg_path[50] = {};
	strcat(cfg_path, NYX_DATABASE_PATH_CFG_FILES);
	strcat(cfg_path, id);
	strcat(cfg_path, ".cfg");
	int fd = creat(cfg_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		perror("creat() fail\n");
		printf("errno: %d\n", errno);
		exit(1);
	}
	close(fd);
}