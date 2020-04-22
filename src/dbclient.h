#pragma once

#ifndef DBCLIENT_H
#define DBCLIENT_H

#include "AbisRest.h"

#define SQL_SELECT_BC_TMPS  "SELECT * FROM t_biocards bc, t_biocard_template_link bt WHERE bc.gid=$1::uuid AND bc.uid = bt.biocard_id"
#define SQL_SELECT_BIOCARDS "SELECT * FROM t_biocards bc where bc.gid=$1::uuid"

#define DB_HOST     "localhost"
#define DB_PORT     "5432"
#define DB_DATABASE "dbABIS"
#define DB_USER     "postgres"
#define DB_PWD      "postgres"

void DBprepare();

PGconn* DBstart();
void DBfinish(PGconn* db);

bool DBstatus(PGconn* db, const ConnStatusType status);
void DBassert(PGconn* db, const ConnStatusType status, const string& error);

bool DBstatus(PGresult* res, const ExecStatusType status);
void DBassert(PGresult* res, const ExecStatusType status, const string& error);
void DBassert(PGresult* res, const ExecStatusType status, const string& error, const string& info);

/*
запрос без проверки результата выполнения
*/
PGresult* DBexec(PGconn* db, const char* query, const char* params[]);

/*
запрос который ничего не возвращает
*/
void DBexec0(PGconn* db, const char* query, const char* params[]);

/*
запрос который возвращает строки
*/
PGresult* DBexec2(PGconn* db, const char* query, const char* params[]);

#endif
