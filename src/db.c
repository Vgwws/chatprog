#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env_parser.h"
#include "db.h"

PGconn* conn;

unsigned long dj2b_hash(const char* str){
	unsigned long hash = 5381;
	for(unsigned int i = 0; str[i]; i++)
		hash = ((hash << 5) + hash) + str[i];
	return hash;
}

u8 db_error_check(PGresult* result, const char* error_message, 
		ExecStatusType flag){
	if(PQresultStatus(result) != flag){
		fprintf(stderr, "ERROR: %s '%s'\n", error_message, PQerrorMessage(conn));
		PQclear(result);
		if(PQstatus(conn) != CONNECTION_OK){
			fprintf(stderr, "ERROR: Bad connection\n");
			db_exit();
			return 1;
		}
	}
	return 0;
}

PGconn* db_init(void){
	char* host = dotenv_get("DB_HOST");
	char* port = dotenv_get("DB_PORT");
	char* name = dotenv_get("DB_NAME");
	char* user = dotenv_get("DB_USER");
	char* password = dotenv_get("DB_PASSWORD");
	char* sslmode = dotenv_get("DB_SSLMODE");
	char conninfo[BUFFER_SIZE];
	snprintf(
			conninfo,
			sizeof(conninfo),
			"host=%s "
			"port=%s "
			"dbname=%s "
			"user=%s "
			"password=%s "
			"sslmode=%s",
			host, port, name, user, password, sslmode);
	free(host);
	free(port);
	free(name);
	free(user);
	free(password);
	free(sslmode);
	conn = PQconnectdb(conninfo);
	if(PQstatus(conn) != CONNECTION_OK){
		fprintf(stderr, "ERROR: Can't connect to database '%s'\n", 
				PQerrorMessage(conn));
		PQfinish(conn);
		return NULL;
	}
	PGresult* result = PQexec(
			conn,
			"CREATE TABLE IF NOT EXISTS users ("
			"name TEXT NOT NULL, "
			"password BIGINT"
			");");
	if(db_error_check(result, "Can't create table for users", PGRES_COMMAND_OK))
		return NULL;
	PQclear(result);
	return conn;
}

void db_exit(void){
	if(conn)
		PQfinish(conn);
	conn = NULL;
}

u8 db_insert_new_user(const char* name, const char* password){
	char hash[32];
	snprintf(hash, sizeof(hash), "%lu", dj2b_hash(password));
	const char* param_values[2] = {name, hash};
	PGresult* result = PQexecParams(
			conn,
			"SELECT name FROM users WHERE name = $1",
			1,
			NULL,
			param_values,
			NULL,
			NULL,
			0);
	if(db_error_check(result, "Can't select username", PGRES_TUPLES_OK))
		return 1;
	if(PQntuples(result)){
		fprintf(stderr, "ERROR: Username already being taken\n");
		PQclear(result);
		return 2;
	}
	result = PQexecParams(
			conn,
			"INSERT INTO users (name, password) VALUES ($1, $2)",
			2,
			NULL,
			param_values,
			NULL,
			NULL,
			0);
	if(db_error_check(result, "Can't insert new user", PGRES_COMMAND_OK))
		return 1;
	PQclear(result);
	return 0;
}

u8 db_start_chat(void){
	PGresult* result = PQexec(
			conn,
			"CREATE TABLE IF NOT EXISTS chat ("
			"username TEXT NOT NULL, "
			"message TEXT NOT NULL, "
			"time TIMESTAMPTZ DEFAULT now()"
			");");
	if(db_error_check(result, "Can't create table for chat", PGRES_COMMAND_OK))
		return 1;
	PQclear(result);
	return 0;
}

u8 db_insert_new_message(const char* username, const char* message){
	const char* param_values[2] = {username, message};
	PGresult* result = PQexecParams(
			conn,
			"INSERT INTO chat (username, message) VALUES ($1, $2)",
			2,
			NULL,
			param_values,
			NULL,
			NULL,
			0);
	if(db_error_check(result, "Can't insert new message", PGRES_COMMAND_OK))
		return 1;
	PQclear(result);
	return 0;
}

PGresult* db_get_password(const char* username){
	const char* param_values[1] = {username};
	PGresult* result = PQexecParams(
			conn,
			"SELECT password FROM users WHERE name = $1",
			1,
			NULL,
			param_values,
			NULL,
			NULL,
			0);
	if(db_error_check(result, "Can't select password", PGRES_TUPLES_OK))
		return NULL;
	return result;
}

u8 db_user_login(const char* username, const char* password){
	PGresult* result = db_get_password(username);
	if(!result)
		return 1;
	if(!PQntuples(result)){
		if(db_insert_new_user(username, password))
			return 1;
		return 0;
	}
	unsigned long hash = strtoull(PQgetvalue(result, 0, 0), NULL, 10);
	PQclear(result);
	if(dj2b_hash(password) != hash){
		fprintf(stderr, "ERROR: Invalid password\n");
		return 2;
	}
	return 0;
}
