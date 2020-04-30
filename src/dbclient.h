#pragma once

#ifndef DBCLIENT_H
#define DBCLIENT_H

#include "AbisRest.h"

#define SEARCH_FACE_PARTS 8
#define SEARCH_FACE_PARAMS "face_vectors_params"
#define SEARCH_FACE_INDEXS "face_vectors_index"
#define SEARCH_FACE_VECTORS "face_vectors"

#define SQL_TMP_IDS_BY_BC_GID "SELECT * FROM t_biocard_template_link bt \
                                JOIN t_biocards bc ON bc.uid = bt.biocard_id AND bc.gid = $1::uuid"

#define SQL_FACE_TMPS_BY_BC_GID   "SELECT * FROM face_vectors fv \
                                    JOIN t_biocard_template_link bt ON bt.tmp_id = fv.id  AND bt.tmp_type = 17 \
                                    JOIN t_biocards bc ON bc.uid = bt.biocard_id AND bc.gid = $1::uuid"

#define SQL_FACE_TMP_SEQ    "SELECT nextval('template_face_seq'::regclass) uid"

#define SQL_LINKS_BY_TMP_ID "SELECT bc.* FROM t_biocard_template_link bt \
                              JOIN t_biocards bc ON bc.uid = bt.biocard_id \
                             WHERE bt.tmp_type = $1::integer AND bt.tmp_id = $2::integer"

#define SQL_BCS_BY_GID      "SELECT * FROM t_biocards bc WHERE bc.gid=$1::uuid"
#define SQL_FACETMP_BY_ID   "SELECT * FROM face_vectors fv WHERE fv.id=$1::integer"

#define SQL_SEARCH_FACE_TMPS "SELECT search_data($1::real[], $2::integer, $3::integer, $4, $5, $6, $7)"
#define SQL_INSERT_FACE_TMP  "SELECT insert_data($1::real[], $2::integer, $3::integer, $4, $5, $6, $7)"

#define SQL_ADD_BC      "INSERT INTO  t_biocards (gid, info) VALUES ($1::uuid, $2) RETURNING uid"
#define SQL_ADD_LINK    "INSERT INTO  t_biocard_template_link (tmp_type, tmp_id, biocard_id) \
                            VALUES ($1::integer, $2::integer, $3::integer)"

#ifdef _HOME
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

//functions for byte swaping
#define ByteSwap(x)  byteswap((unsigned char*) &x, sizeof(x))
#define byteSwap(x,n)  byteswap((unsigned char*) &x,  n)

template<class T>
void db_get_array(T*& ar, char* mem);
void db_get_array(char*& ar, char* mem);
void db_get_array(char**& ar, char* mem);

void db_prepare();

PGconn* db_open();
void db_close(PGconn* db);

void db_tx_begin(PGconn* db);
void db_tx_commit(PGconn* db);
void db_tx_rollback(PGconn* db);

void db_sp_begin(PGconn* db, const char* name);
void db_sp_rollback(PGconn* db, const char* name);
void db_sp_release(PGconn* db, const char* name);

/*
поиск id шаблона по графу
*/
int db_search_face_tmp(PGconn* db, const void* tmp_arr);

/*
reserv
*/
int db_search_finger_tmp(PGconn* db, const void* tmp_arr);

/*
вставка шаблона лица в таблицу поиска
*/
int db_insert_face_tmp(PGconn* db, const void* tmp_arr, int tmp_id);

/*
поиск gid биокарты по id шаблона
*/
int db_card_id_by_tmp_id(PGconn* db, int tmp_type, int tmp_id, char* gid);

/*
получение ИД биокарты по uuid
*/
int db_card_id_by_gid(PGconn* db, const char* gid);

/*
получение шаблона по его ИД
*/
int db_face_tmp_by_id(PGconn* db, int tmp_id, void*& tmp_arr);

/*
добавляем в базу биокарту 
*/
int db_add_bc(PGconn* db, const char* gid, const char* info);

/*
добавляем в базу связь шаблон-биокарта
*/
int db_add_link(PGconn* db, int tmp_type, int tmp_id, int bc_id);


int db_get_face_seq(PGconn* db);



#endif