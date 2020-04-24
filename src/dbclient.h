#pragma once

#ifndef DBCLIENT_H
#define DBCLIENT_H

#include "AbisRest.h"

#define SEARCH_FACE_PARTS   8
#define SEARCH_FACE_PARAMS  "face_vectors_params"
#define SEARCH_FACE_INDEXS  "face_vectors_index"
#define SEARCH_FACE_VECTORS "face_vectors"

#define SQL_LINKS_BY_BC_GID "SELECT * FROM t_biocards bc, t_biocard_template_link bt WHERE bc.gid = $1::uuid AND \
    bc.uid = bt.biocard_id"

#define SQL_LINKS_BY_TMP_ID "SELECT * FROM t_biocards bc, t_biocard_template_link bt WHERE bt.tmp_type = $1::integer AND \
    bt.tmp_id = $2::integer  AND bc.uid = bt.biocard_id"

#define SQL_BCS_BY_GID      "SELECT * FROM t_biocards bc where bc.gid=$1::uuid"

#define SQL_SEARCH_FACE_TMPS "SELECT search_data($1::real[], $2::integer, $3::integer, $4, $5, $6, $7)"

#ifdef _WIN32
#define DB_HOST     "localhost"
#define DB_PORT     "5432"
#define DB_DATABASE "dbABIS"
#define DB_USER     "postgres"
#define DB_PWD      "postgres"
#else
#define DB_HOST     "10.6.46.130"
#define DB_PORT     "5432"
#define DB_DATABASE "dbABIS"
#define DB_USER     "postgres"
#define DB_PWD      "g6nsgWG2Xk"
#endif

void db_prepare();

PGconn* db_open();
void db_close(PGconn* db);

/*
����� id ������� �� �����
*/
int db_search_face_template(PGconn* db, const void* tmp_arr);

/*
reserv
*/
int db_search_finger_template(PGconn* db, const void* tmp_arr);

/*
����� gid �������� �� id �������
*/
int db_find_biocard_by_template(PGconn* db, int tmp_type, int tmp_id, char* gid);


#endif