#define NYX_DATABASE_ACCOUNT_CFG_FILE "./bin/cfg/accounts.cfg"
#define NYX_DATABASE_PATH_CFG_FILES "./bin/cfg/"


void  Nyx_database_init();
int   Nyx_database_account_lookupID(char *id);
int   Nyx_database_passwork_check(char *id, char *password);
void  Nyx_database_userlist_get(char *userlist);
void  Nyx_database_chat_getbyuser(char *chatlist, char *id);
void  Nyx_database_chat_dialog_get(char *dialog, char *id, char *chat_title);
void  Nyx_database_chat_message_append(char *id, char *chat_title, char *message);
void  Nyx_database_account_new(char *id, char *password);