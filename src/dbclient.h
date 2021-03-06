﻿#pragma once

#ifndef DBCLIENT_H
#define DBCLIENT_H

#include "AbisRest.h"

static const string SQL_TMP_IDS_BY_BC_GID("SELECT * FROM t_biocard_template_link bt \
                                JOIN t_biocards bc ON bc.uid = bt.biocard_id AND bc.gid = $1::uuid");

static const string SQL_FACE_TMP_SEQ("SELECT nextval('template_face_seq'::regclass) uid");
static const string SQL_FINGER_TMP_SEQ("SELECT nextval('template_finger_seq'::regclass) uid");
static const string SQL_IRIS_TMP_SEQ("SELECT nextval('template_iris_seq'::regclass) uid");

static const string SQL_TMP_BY_ID("SELECT * FROM %1% tmp WHERE tmp.id=$1::integer");

static const string SQL_SEARCH_TMPS("SELECT search_data($1::real[], $2::integer, $3::integer, $4, $5, $6, $7)");
static const string SQL_INSERT_TMP("SELECT insert_data($1::real[], $2::integer, $3::integer, $4, $5, $6, $7)");
static const string SQL_UPDATE_FINGER("UPDATE %1% SET fpos = $1 WHERE id = $2 RETURNING *");
static const string SQL_ADDGOST_FINGER("UPDATE %1% SET gv = $1::bytea WHERE id = $2 RETURNING *");

static const string SQL_ADD_BC("INSERT INTO  t_biocards (gid, info) VALUES ($1::uuid, $2) RETURNING uid");
static const string SQL_GET_BC_BY_GID("SELECT * FROM t_biocards bc WHERE bc.gid=$1::uuid");
static const string SQL_DEL_BC_BY_ID("DELETE FROM t_biocards bc WHERE bc.uid = $1::integer RETURNING *");
static const string SQL_DEL_BC_BY_GID("DELETE FROM t_biocards bc WHERE bc.gid = $1::uuid RETURNING *");

static const string SQL_ADD_LINK("INSERT INTO  t_biocard_template_link (tmp_type, tmp_id, biocard_id) \
                            VALUES ($1::integer, $2::integer, $3::integer)");
static const string SQL_LINKS_BY_TMP_ID("SELECT bc.* FROM t_biocard_template_link bt \
                              JOIN t_biocards bc ON bc.uid = bt.biocard_id \
                             WHERE bt.tmp_type = $1::integer AND bt.tmp_id = $2::integer");
static const string SQL_DEL_LINK("DELETE FROM t_biocard_template_link bt WHERE bt.uid = ( \
                            SELECT bt.uid FROM t_biocard_template_link bt \
                                JOIN t_biocards bc ON bc.uid = bt.biocard_id \
                            WHERE bt.tmp_type = $1::integer AND bt.tmp_id = $2::integer AND bc.gid = $3::uuid \
                         ) RETURNING * ");
static const string SQL_DEL_LINKS_BY_ID("DELETE FROM t_biocard_template_link bt WHERE bt.uid = $1::integer RETURNING *");
static const string SQL_DEL_LINKS_BY_GID("DELETE FROM t_biocard_template_link bt WHERE bt.biocard_id = (\
                            SELECT bc.uid FROM t_biocards bc WHERE bc.gid = $1::uuid \
						) RETURNING *");

#define ABIS_SEARCH_COUNT   10

//functions for byte swaping
#define ByteSwap(x)  byteswap((unsigned char*) &x, sizeof(x))
#define byteSwap(x,n)  byteswap((unsigned char*) &x,  n)

template<class T>
void db_get_array(T*& ar, char* mem);
template<class T>
void db_get_array(vector<T>& arr, char* mem);
void db_get_array(char*& ar, char* mem);
void db_get_array(char**& ar, char* mem);

void logging_res(const string fname, PGresult* sql_res);

void db_prepare();

PGconn* db_open();
void db_close(PGconn* db);
PGresult* db_exec_param(PGconn* db, string sql, const int nParams, const char* const* params, const int resFormat);

void db_tx_begin(PGconn* db);
void db_tx_commit(PGconn* db);
void db_tx_rollback(PGconn* db);

void db_sp_begin(PGconn* db, const char* name);
void db_sp_rollback(PGconn* db, const char* name);
void db_sp_release(PGconn* db, const char* name);

int db_get_face_seq(PGconn* db);
int db_get_finger_seq(PGconn* db);
int db_get_iris_seq(PGconn* db);
int db_get_sequence(PGconn* db, const int tmp_type);

/*
шаблоны
*/
int db_search_face_tmps(PGconn* db, const void* tmp_arr, const int count, vector<int>& ids);
int db_search_face_tmp(PGconn* db, const void* tmp_arr, int& id);
int db_insert_face_tmp(PGconn* db, const void* tmp_arr, const int tmp_id);
int db_face_tmp_by_id(PGconn* db, const int tmp_id, const void* tmp_arr);

int db_search_finger_tmps(PGconn* db, const void* tmp_arr, const int count, vector<int>& ids);
int db_search_finger_tmp(PGconn* db, const void* tmp_arr, int& id);
int db_insert_finger_tmp(PGconn* db, const void* tmp_arr, const int tmp_id);
int db_append_finger_gost(PGconn* db, const void* tmp_arr, const int tmp_id);
int db_set_finger_num(PGconn* db, const int tmp_id, const int fnum);
int db_finger_tmp_by_id(PGconn* db, const int tmp_id, const void* tmp_arr);
int db_gost_tmp_by_id(PGconn* db, const int tmp_id, const void* tmp_arr);

int db_search_iris_tmps(PGconn* db, const void* tmp_arr, const int count, vector<int>& ids);
int db_search_iris_tmp(PGconn* db, const void* tmp_arr, int& id);
int db_insert_iris_tmp(PGconn* db, const void* tmp_arr, const int tmp_id);
int db_iris_tmp_by_id(PGconn* db, const int tmp_id, const void* tmp_arr);

int db_tmp_cmp_by_id(PGconn* db, const int tmp_type, const void* tmp_ptr, const int tmp_id, float& score);

/*
биокарты
*/
int db_add_bc(PGconn* db, const char* gid, const char* info);
int db_del_bc(PGconn* db, const int bc_id);
int db_del_bc(PGconn* db, const char* gid);
int db_get_bc_by_gid(PGconn* db, const char* gid);
int db_get_bc_for_tmp(PGconn* db, const int tmp_type, const int tmp_id, char* gid);

/*
связи
*/
int db_add_link(PGconn* db, const int tmp_type, const int tmp_id, const int bc_id);
int db_del_link(PGconn* db, const int tmp_type, const int tmp_id, const char* gid);
int db_del_links(PGconn* db, const int bc_id);
int db_del_links(PGconn* db, const char* gid);

#endif