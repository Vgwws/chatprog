#ifndef DB_H
#define DB_H

#include <libpq-fe.h>

#include "common.h"

u8 db_init(void);
void db_exit(void);
u8 db_start_chat(void);
u8 db_insert_new_message(const char* username, const char* password);
u8 db_user_login(const char* username, const char* password);

#endif
