#pragma once

#ifndef DBCLIENT_H
#define DBCLIENT_H

#include "AbisRest.h"

#define SEARCH_FACE_PARTS 8
#define SEARCH_FACE_PARAMS "face_vectors_params"
#define SEARCH_FACE_INDEXS "face_vectors_index"
#define SEARCH_FACE_VECTORS "face_vectors"

#define SQL_TMP_IDS_BY_BC_GID "SELECT bt.* FROM t_biocards bc, t_biocard_template_link bt \
                                WHERE bc.gid = $1::uuid AND bc.uid = bt.biocard_id"

#define SQL_FACE_TMPS_BY_BC_GID   "SELECT * FROM face_vectors fv \
                                    JOIN t_biocard_template_link bt ON bt.tmp_id = fv.id  AND bt.tmp_type = 17 \
                                    JOIN t_biocards bc ON bc.uid = bt.biocard_id AND bc.gid = $1::uuid"

#define SQL_LINKS_BY_TMP_ID "SELECT * FROM t_biocards bc, t_biocard_template_link bt \
                             WHERE bt.tmp_type = $1::integer AND bt.tmp_id = $2::integer AND bc.uid = bt.biocard_id"

#define SQL_BCS_BY_GID "SELECT * FROM t_biocards bc where bc.gid=$1::uuid"

#define SQL_TMP_BY_ID "SELECT * FROM face_vectors fv WHERE fv.id=$1::integer"

#define SQL_SEARCH_FACE_TMPS "SELECT search_data($1::real[], $2::integer, $3::integer, $4, $5, $6, $7)"

#ifdef _WIN32
#define DB_HOST "localhost"
#define DB_PORT "5432"
#define DB_DATABASE "dbABIS"
#define DB_USER "postgres"
#define DB_PWD "postgres"
#else
#define DB_HOST "10.6.46.130"
#define DB_PORT "5432"
#define DB_DATABASE "dbABIS"
#define DB_USER "postgres"
#define DB_PWD "g6nsgWG2Xk"
#endif

void db_prepare();

PGconn* db_open();
void db_close(PGconn* db);

/*
поиск id шаблона по графу
*/
int db_search_face_tmp(PGconn* db, const void* tmp_arr);

/*
reserv
*/
int db_search_finger_tmp(PGconn* db, const void* tmp_arr);

/*
поиск gid биокарты по id шаблона
*/
int db_find_biocard_by_tmp_id(PGconn* db, int tmp_type, int tmp_id, char* gid);

/*
получение шаблона по его ИД
*/
int db_get_tmp_by_id(PGconn* db, int tmp_type, int tmp_id, void* tmp_arr);

//functions for byte swaping
#define ByteSwap(x)  byteswap((unsigned char*) &x, sizeof(x))
#define byteSwap(x,n)  byteswap((unsigned char*) &x,  n)

template<class T>
void db_get_array(T*& ar, char* mem);
void db_get_array(char*& ar, char* mem);
void db_get_array(char**& ar, char* mem);

#endif