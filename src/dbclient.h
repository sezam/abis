#pragma once

#ifndef DBCLIENT_H
#define DBCLIENT_H

#include "AbisRest.h"

static const string SQL_TMP_IDS_BY_BC_GID("SELECT * FROM t_biocard_template_link bt \
                                JOIN t_biocards bc ON bc.uid = bt.biocard_id AND bc.gid = $1::uuid");

static const string SQL_FACE_TMP_SEQ("SELECT nextval('template_face_seq'::regclass) uid");
static const string SQL_FINGER_TMP_SEQ("SELECT nextval('template_finger_seq'::regclass) uid");

static const string SQL_LINKS_BY_TMP_ID("SELECT bc.* FROM t_biocard_template_link bt \
                              JOIN t_biocards bc ON bc.uid = bt.biocard_id \
                             WHERE bt.tmp_type = $1::integer AND bt.tmp_id = $2::integer");

static const string SQL_BCS_BY_GID("SELECT * FROM t_biocards bc WHERE bc.gid=$1::uuid");
static const string SQL_TMP_BY_ID("SELECT * FROM %1% fv WHERE fv.id=$1::integer");

static const string SQL_SEARCH_TMPS("SELECT search_data($1::real[], $2::integer, $3::integer, $4, $5, $6, $7)");
static const string SQL_INSERT_TMP("SELECT insert_data($1::real[], $2::integer, $3::integer, $4, $5, $6, $7)");
static const string SQL_UPDATE_FINGER("UPDATE %1% SET fpos = $1 WHERE id = $2 RETURNING *");
static const string SQL_ADDGOST_FINGER("UPDATE %1% SET vgost = $1::bytea WHERE id = $2 RETURNING *");

static const string SQL_ADD_BC("INSERT INTO  t_biocards (gid, info) VALUES ($1::uuid, $2) RETURNING uid");
static const string SQL_ADD_LINK("INSERT INTO  t_biocard_template_link (tmp_type, tmp_id, biocard_id) \
                            VALUES ($1::integer, $2::integer, $3::integer)");
static const string SQL_DEL_LINK("DELETE FROM t_biocard_template_link bt WHERE bt.uid = ( \
                            SELECT bt.uid FROM t_biocard_template_link bt \
                                JOIN t_biocards bc ON bc.uid = bt.biocard_id \
                            WHERE bt.tmp_type = $1::integer AND bt.tmp_id = $2::integer AND bc.gid = $3::uuid \
                         ) RETURNING * ");

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
PGresult* db_exec_param(PGconn* db, string sql, int nParams, const char* const* params, int resFormat);

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
int db_insert_finger_tmp(PGconn* db, const void* tmp_arr, int tmp_id);

int db_append_finger_gost(PGconn* db, const void* tmp_arr, int tmp_id);
int db_set_finger_num(PGconn* db, int tmp_id, int fnum);

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
int db_finger_tmp_by_id(PGconn* db, int tmp_id, void*& tmp_arr);
int db_gost_tmp_by_id(PGconn* db, int tmp_id, void*& tmp_arr);

/*
добавляем в базу биокарту 
*/
int db_add_bc(PGconn* db, const char* gid, const char* info);

/*
добавляем в базу связь шаблон-биокарта
*/
int db_add_link(PGconn* db, int tmp_type, int tmp_id, int bc_id);

/*
удаляем связь шаблон-биокарта
*/
int db_del_link(PGconn* db, int tmp_type, int tmp_id, const char* gid);

int db_get_face_seq(PGconn* db);
int db_get_finger_seq(PGconn* db);

#endif