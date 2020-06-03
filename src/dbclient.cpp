#include "AbisRest.h"
#include "dbclient.h"
#include "ebsclient.h"
#include "fplibclient.h"

// a helper function to get the number of elements in an array
int getNoEle(char* m)
{
	return ntohl(*(int*)(m + 3 * sizeof(int)));
}

void byteswap(unsigned char* b, int n)
{
	register int i = 0;
	register int j = n - 1;
	while (i < j)
	{
		std::swap(b[i], b[j]);
		i++, j--;
	}
}

template<class T>
void db_get_array(T*& ar, char* mem)
{
	assert(ar != nullptr);
	assert(mem != nullptr);

	size_t nEle = getNoEle(mem);
	size_t intSize = sizeof(int);

	char* start = mem + 5 * intSize;
	for (size_t i = 0; i < nEle; i++)
	{
		size_t size = pg_ntoh32(*(int*)(start));
		ar[i] = (*(T*)(start + intSize));
		byteSwap(ar[i], size);
		start += (size_t)(size + intSize);
	}
	BOOST_LOG_TRIVIAL(debug) << "db_get_array: el_count = " << nEle << " el_size = " << sizeof(T);
}

void db_get_array(char*& ar, char* mem)
{
	assert(ar != nullptr);
	assert(mem != nullptr);

	size_t nEle = getNoEle(mem);
	size_t intSize = sizeof(int);

	char* start = mem + 5 * intSize;
	for (size_t i = 0; i < nEle; i++)
	{
		int size = pg_ntoh32(*(int*)(start));
		ar[i] = *(char*)(start + intSize);
		start += (size_t)(size + intSize);
	}

	BOOST_LOG_TRIVIAL(debug) << "db_get_array: el_count = " << nEle << " el_size = " << sizeof(char);
}

/*
!!! выделение памяти
*/
void db_get_array(char**& ar, char* mem)
{
	assert(ar != nullptr);
	assert(mem != nullptr);

	size_t nEle = getNoEle(mem);
	size_t intSize = sizeof(int);

	char* start = mem + 5 * intSize;
	for (size_t i = 0; i < nEle; i++)
	{
		size_t size = pg_ntoh32(*(int*)(start));
		ar[i] = new char[size]; //!!!
		strncpy(ar[i], (char*)(start + intSize), size + 1);
		start += (size_t)(size + intSize);
	}
	BOOST_LOG_TRIVIAL(debug) << "db_get_array: el_count = " << nEle << " el_size = " << sizeof(char*);
}

void logging_res(string fname, PGresult* sql_res)
{
	stringstream ss;
	ss << fname << ": result = " << PQresStatus(PQresultStatus(sql_res));
	ss << " tuples = " << PQcmdTuples(sql_res);
	if (PQresultErrorMessage(sql_res) != nullptr) ss << " msg = " << PQresultErrorMessage(sql_res);
	ss << endl;

	if (PQntuples(sql_res) > 0)
	{
		for (size_t i = 0; i < PQnfields(sql_res); i++)
		{
			ss << PQfname(sql_res, i);
			ss << ": len = " << PQgetlength(sql_res, 0, i);
			ss << ", type = " << PQftype(sql_res, i);
			if (PQgetlength(sql_res, 0, i) < 10)
			{
				ss << ", value = ";
				if (PQfformat(sql_res, i) == 0)
					ss << PQgetvalue(sql_res, 0, i);
				else
					ss << ntohl(*((uint32_t*)PQgetvalue(sql_res, 0, i)));
			}
			ss << endl;
		}
	}
	BOOST_LOG_TRIVIAL(debug) << ss.str();
}

void logging_conn(string fname, PGconn* db)
{
	BOOST_LOG_TRIVIAL(debug) << fname << ": msg = " << PQerrorMessage(db);
}

void db_prepare()
{
	PGconn* db = db_open();

	BOOST_LOG_TRIVIAL(info) << "DB connection success \t" << PQuser(db) << ":" << PQpass(db)
		<< "@" << PQhost(db) << ":" << PQport(db) << "/" << PQdb(db);

	db_close(db);
}

PGconn* db_open()
{
	PGconn* db = PQsetdbLogin(postgres_host.c_str(), postgres_port.c_str(), "", "",
		postgres_db.c_str(), postgres_user.c_str(), postgres_pwd.c_str());
	logging_conn("db_open", db);

	if (PQstatus(db) != CONNECTION_OK) throw runtime_error("Error connection db.");

	return db;
}

void db_close(PGconn* db)
{
	logging_conn("db_close", db);
	PQfinish(db);
}

PGresult* db_exec_param(PGconn* db, string sql, int nParams, const char* const* params, int resFormat)
{
	return PQexecParams(db, sql.c_str(), nParams, nullptr, params, nullptr, nullptr, resFormat);
}

void db_tx_begin(PGconn* db)
{
	PGresult* res = PQexec(db, "BEGIN");
	logging_res("db_tx_begin", res);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) throw runtime_error("db_tx_begin: error open transaction");
	PQclear(res);
}

void db_tx_commit(PGconn* db)
{
	PGresult* res = PQexec(db, "COMMIT");
	logging_res("db_tx_commit", res);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) throw runtime_error("db_tx_commit: error commit transaction");
	PQclear(res);
}

void db_tx_rollback(PGconn* db)
{
	PGresult* res = PQexec(db, "ROLLBACK");
	logging_res("db_tx_rollback", res);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) throw runtime_error("db_tx_rollback: error rollback transaction");
	PQclear(res);
}

void db_sp_begin(PGconn* db, const char* name)
{
	string q("SAVEPOINT ");
	q.append(name);

	PGresult* res = PQexec(db, q.c_str());
	logging_res("db_sp_begin", res);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) throw runtime_error("db_sp_begin: error open savepoint");
	PQclear(res);
}

void db_sp_rollback(PGconn* db, const char* name)
{
	string q("ROLLBACK TO SAVEPOINT ");
	q.append(name);

	PGresult* res = PQexec(db, q.c_str());
	logging_res("db_sp_rollback", res);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) throw runtime_error("db_sp_rollback: error rollback savepoint");
	PQclear(res);
}

void db_sp_release(PGconn* db, const char* name)
{
	string q("RELEASE SAVEPOINT ");
	q.append(name);

	PGresult* res = PQexec(db, q.c_str());
	logging_res("db_sp_release", res);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) throw runtime_error("db_sp_release: error release savepoint");
	PQclear(res);
}

int db_search_tmp(PGconn* db, const void* tmp_arr, string index_name, string param_name, string vector_name)
{
	assert(tmp_arr != nullptr);

	int result = 0;

	string arr("{");
	for (size_t i = 0; i < ABIS_TEMPLATE_LEN; i++)
	{
		arr.append(to_string(((float*)tmp_arr)[i]));
		if (i != ABIS_TEMPLATE_LEN - 1) arr.append(",");
	}
	arr.append("}");

	string part_size_s = to_string(ABIS_TEMPLATE_LEN / face_parts);

	const char* paramValues[7] = { arr.c_str(),  part_size_s.c_str(), "1",
		postgres_db.c_str(), param_name.c_str(), index_name.c_str(), vector_name.c_str() };

	PGresult* sql_res = nullptr;
	try
	{
		sql_res = db_exec_param(db, SQL_SEARCH_TMPS, 7, paramValues, 1);
		logging_res("db_search_tmp", sql_res);

		if (PQresultStatus(sql_res) == PGRES_TUPLES_OK && PQntuples(sql_res) > 0)
		{
			int value[5];
			int* arr_ptr = value;
			char* res_ptr = PQgetvalue(sql_res, 0, 0);
			if (PQgetlength(sql_res, 0, 0) > 0) {
				db_get_array(arr_ptr, res_ptr);
				result = arr_ptr[0];
			}
		}
	}
	catch (const std::exception& ec)
	{
		BOOST_LOG_TRIVIAL(debug) << PQcmdStatus(sql_res);
		BOOST_LOG_TRIVIAL(error) << "db_search_tmp: " << ec.what();
		result = -1;
	}

	PQclear(sql_res);
	return result;
}


int db_search_face_tmp(PGconn* db, const void* tmp_arr)
{
	return db_search_tmp(db, tmp_arr, face_index, face_param, face_vector);
}

int db_search_finger_tmp(PGconn* db, const void* tmp_arr)
{
	return db_search_tmp(db, tmp_arr, finger_index, finger_param, finger_vector);
}

int db_insert_tmp(PGconn* db, const void* tmp_arr, int tmp_id, string index_name, string param_name, string vector_name)
{
	assert(tmp_arr != nullptr);
	assert(tmp_id > 0);

	int result = 0;

	string arr("{");
	for (size_t i = 0; i < ABIS_TEMPLATE_LEN; i++)
	{
		arr.append(to_string(((float*)tmp_arr)[i]));
		if (i != ABIS_TEMPLATE_LEN - 1) arr.append(",");
	}
	arr.append("}");

	string tmp_id_s = to_string(tmp_id);
	string part_size_s = to_string(ABIS_TEMPLATE_LEN / face_parts);
	const char* paramValues[7] = { arr.c_str(),  tmp_id_s.c_str(), part_size_s.c_str(),
		postgres_db.c_str(), param_name.c_str(), index_name.c_str(), vector_name.c_str() };

	PGresult* sql_res = nullptr;
	try
	{
		sql_res = db_exec_param(db, SQL_INSERT_TMP, 7, paramValues, 1);
		logging_res("db_insert_tmp", sql_res);


		if (PQresultStatus(sql_res) == PGRES_TUPLES_OK) result = 1;
	}
	catch (const std::exception& ec)
	{
		BOOST_LOG_TRIVIAL(debug) << PQcmdStatus(sql_res);
		BOOST_LOG_TRIVIAL(error) << "db_insert_tmp: " << ec.what();
		result = -1;
	}

	PQclear(sql_res);
	return result;
}

int db_insert_face_tmp(PGconn* db, const void* tmp_arr, int tmp_id)
{
	return db_insert_tmp(db, tmp_arr, tmp_id, face_index, face_param, face_vector);
}

int db_insert_finger_tmp(PGconn* db, const void* tmp_arr, int tmp_id)
{
	return db_insert_tmp(db, tmp_arr, tmp_id, finger_index, finger_param, finger_vector);
}

int db_append_finger_gost(PGconn* db, const void* tmp_arr, int tmp_id)
{
	assert(tmp_arr != nullptr);
	assert(tmp_id > 0);

	int result = 0;
	int esz = base64::encoded_size(ABIS_FINGER_TMP_GOST_SIZE);

	unsigned char* b64 = (unsigned char*)malloc( esz + 1);
	if (b64 != nullptr)
	{
		size_t res = base64::encode(b64, tmp_arr, ABIS_FINGER_TMP_GOST_SIZE);
		b64[esz] = 0;

		string s1 = to_string(tmp_id);
		const char* paramValues[2] = { (const char*)b64, s1.c_str() };

		PGresult* sql_res = nullptr;
		try
		{
			sql_res = db_exec_param(db, str(boost::format(SQL_ADDGOST_FINGER) % finger_vector), 2, paramValues, 1);
			logging_res("db_append_finger_gost", sql_res);

			if (PQresultStatus(sql_res) == PGRES_TUPLES_OK && PQntuples(sql_res) > 0)
			{
				int id_num = PQfnumber(sql_res, "id");
				result = pg_ntoh32(*(int*)(PQgetvalue(sql_res, 0, id_num)));
			}
		}
		catch (const std::exception& ec)
		{
			BOOST_LOG_TRIVIAL(debug) << PQcmdStatus(sql_res);
			BOOST_LOG_TRIVIAL(error) << "db_append_finger_gost: " << ec.what();
			result = -1;
		}

		PQclear(sql_res);
		free(b64);
	}
	return result;
}

int db_set_finger_num(PGconn* db, int tmp_id, int fnum)
{
	assert(tmp_id > 0);

	int result = 0;

	string s1 = to_string(tmp_id);
	string s2 = to_string(fnum);
	const char* paramValues[2] = { s1.c_str(), s2.c_str() };

	PGresult* sql_res = nullptr;
	try
	{
		sql_res = db_exec_param(db, str(boost::format(SQL_TMP_BY_ID) % finger_vector), 2, paramValues, 1);
		logging_res("db_set_finger_num", sql_res);

		if (PQresultStatus(sql_res) == PGRES_TUPLES_OK && PQntuples(sql_res) > 0)
		{
			int fpos_num = PQfnumber(sql_res, "fpos");
			result = pg_ntoh32(*(int*)(PQgetvalue(sql_res, 0, fpos_num)));
		}
	}
	catch (const std::exception& ec)
	{
		BOOST_LOG_TRIVIAL(debug) << PQcmdStatus(sql_res);
		BOOST_LOG_TRIVIAL(error) << "db_set_finger_num: " << ec.what();
		result = -1;
	}

	PQclear(sql_res);
	return result;
}

int db_tmp_by_id(PGconn* db, int tmp_id, const void* tmp_arr, string table)
{
	assert(tmp_arr != nullptr);
	assert(tmp_id > 0);

	int result = 0;

	string s1 = to_string(tmp_id);
	const char* paramValues[1] = { s1.c_str() };

	PGresult* sql_res = nullptr;
	try
	{
		sql_res = db_exec_param(db, str(boost::format(SQL_TMP_BY_ID) % table), 1, paramValues, 1);
		logging_res("db_tmp_by_id", sql_res);

		if (PQresultStatus(sql_res) == PGRES_TUPLES_OK && PQntuples(sql_res) > 0)
		{
			int arr_num = PQfnumber(sql_res, "vector");
			char* res_ptr = PQgetvalue(sql_res, 0, arr_num);
			result = PQgetlength(sql_res, 0, arr_num);

			float* arr_ptr = (float*)tmp_arr;
			if (result > 0) db_get_array(arr_ptr, res_ptr);
		}
	}
	catch (const std::exception& ec)
	{
		BOOST_LOG_TRIVIAL(debug) << PQcmdStatus(sql_res);
		BOOST_LOG_TRIVIAL(error) << "db_get_tmp_by_id: " << ec.what();
		result = -1;
	}

	PQclear(sql_res);
	return result;
}

int db_gost_tmp_by_id(PGconn* db, int tmp_id, const void* tmp_arr)
{
	assert(tmp_arr != nullptr);
	assert(tmp_id > 0);

	int result = 0;

	string s1 = to_string(tmp_id);
	const char* paramValues[1] = { s1.c_str() };

	PGresult* sql_res = nullptr;
	try
	{
		sql_res = db_exec_param(db, str(boost::format(SQL_TMP_BY_ID) % finger_vector), 1, paramValues, 1);
		logging_res("db_gost_tmp_by_id", sql_res);

		if (PQresultStatus(sql_res) == PGRES_TUPLES_OK && PQntuples(sql_res) > 0)
		{
			int arr_num = PQfnumber(sql_res, "gv");
			const char* res_ptr = PQgetvalue(sql_res, 0, arr_num);
			if (res_ptr != nullptr)
			{
				result = PQgetlength(sql_res, 0, arr_num);
				//unsigned char* arr_ptr = (unsigned char*)tmp_arr;
				//if (result > 0) db_get_array(arr_ptr, res_ptr);
				if (base64::encoded_size(ABIS_FINGER_TMP_GOST_SIZE) == result)
				{
					auto res = base64::decode((void*)tmp_arr, res_ptr, base64::encoded_size(ABIS_FINGER_TMP_GOST_SIZE));
					result = res.first;
				}
			}
			else result = 1;
		}
	}
	catch (const std::exception& ec)
	{
		BOOST_LOG_TRIVIAL(debug) << PQcmdStatus(sql_res);
		BOOST_LOG_TRIVIAL(error) << "db_gost_tmp_by_id: " << ec.what();
		result = -1;
	}

	PQclear(sql_res);
	return result;
}

int db_face_tmp_by_id(PGconn* db, int tmp_id, const void* tmp_arr)
{
	return db_tmp_by_id(db, tmp_id, tmp_arr, face_vector);
}

int db_finger_tmp_by_id(PGconn* db, int tmp_id, const void* tmp_arr)
{
	return db_tmp_by_id(db, tmp_id, tmp_arr, finger_vector);
}

int db_card_id_by_tmp_id(PGconn* db, int tmp_type, int tmp_id, char* gid)
{
	assert(gid != nullptr);
	assert(tmp_id > 0);

	int result = 0;

	string s1 = to_string(tmp_type);
	string s2 = to_string(tmp_id);
	const char* paramValues[2] = { s1.c_str(), s2.c_str() };

	PGresult* sql_res = nullptr;
	try
	{
		sql_res = db_exec_param(db, SQL_LINKS_BY_TMP_ID, 2, paramValues, 1);
		logging_res("db_card_id_by_tmp_id", sql_res);

		if (PQresultStatus(sql_res) == PGRES_TUPLES_OK && PQntuples(sql_res) > 0)
		{
			int gid_num = PQfnumber(sql_res, "gid");
			char* uuid_ptr = PQgetvalue(sql_res, 0, gid_num);
			uuid uid;
			memcpy(&uid, uuid_ptr, 16);
			strcpy(gid, to_string(uid).c_str());

			int uid_num = PQfnumber(sql_res, "uid");
			result = pg_ntoh32(*(int*)(PQgetvalue(sql_res, 0, uid_num)));
		}
	}
	catch (const std::exception& ec)
	{
		BOOST_LOG_TRIVIAL(debug) << PQcmdStatus(sql_res);
		BOOST_LOG_TRIVIAL(error) << "find_biocard_by_template: " << ec.what();
		result = -1;
	}

	PQclear(sql_res);
	return result;
}

int db_card_id_by_gid(PGconn* db, const char* gid)
{
	assert(gid != nullptr);

	int result = 0;
	const char* paramValues[1] = { gid };
	PGresult* sql_res = nullptr;
	try
	{
		sql_res = db_exec_param(db, SQL_BCS_BY_GID, 1, paramValues, 1);
		logging_res("db_card_id_by_gid", sql_res);

		if (PQresultStatus(sql_res) == PGRES_TUPLES_OK && PQntuples(sql_res) > 0)
		{
			int uid_num = PQfnumber(sql_res, "uid");
			result = pg_ntoh32(*(int*)(PQgetvalue(sql_res, 0, uid_num)));
		}
	}
	catch (const std::exception& ec)
	{
		BOOST_LOG_TRIVIAL(debug) << PQcmdStatus(sql_res);
		BOOST_LOG_TRIVIAL(error) << "db_find_bc_by_gid: " << ec.what();
		result = -1;
	}

	PQclear(sql_res);
	return result;
}

int db_add_bc(PGconn* db, const char* gid, const char* info)
{
	assert(gid != nullptr);

	int result = 0;
	const char* paramValues[2] = { gid, info };
	PGresult* sql_res = nullptr;
	try
	{
		sql_res = db_exec_param(db, SQL_ADD_BC, 2, paramValues, 1);
		logging_res("db_add_bc", sql_res);

		if (PQresultStatus(sql_res) == PGRES_TUPLES_OK && PQntuples(sql_res) > 0)
		{
			int uid_num = PQfnumber(sql_res, "uid");
			result = pg_ntoh32(*(int*)(PQgetvalue(sql_res, 0, uid_num)));
		}
	}
	catch (const std::exception& ec)
	{
		BOOST_LOG_TRIVIAL(debug) << PQcmdStatus(sql_res);
		BOOST_LOG_TRIVIAL(error) << "db_add_bc: " << ec.what();
		result = -1;
	}

	PQclear(sql_res);
	return result;
}


int db_add_link(PGconn* db, int tmp_type, int tmp_id, int bc_id)
{
	assert(tmp_id > 0);

	int result = 0;
	string tmp_type_s = to_string(tmp_type);
	string tmp_id_s = to_string(tmp_id);
	string bc_id_s = to_string(bc_id);
	const char* paramValues[3] = { tmp_type_s.c_str(), tmp_id_s.c_str(), bc_id_s.c_str() };
	PGresult* sql_res = nullptr;
	try
	{
		sql_res = db_exec_param(db, SQL_ADD_LINK, 3, paramValues, 1);
		logging_res("db_add_link", sql_res);

		if (PQresultStatus(sql_res) == PGRES_COMMAND_OK) result = 1;
	}
	catch (const std::exception& ec)
	{
		BOOST_LOG_TRIVIAL(debug) << PQcmdStatus(sql_res);
		BOOST_LOG_TRIVIAL(error) << "db_add_link: " << ec.what();
		result = -1;
	}

	PQclear(sql_res);
	return result;
}

int db_del_link(PGconn* db, int tmp_type, int tmp_id, const char* gid)
{
	assert(tmp_id > 0);

	int result = 0;
	string tmp_type_s = to_string(tmp_type);
	string tmp_id_s = to_string(tmp_id);
	const char* paramValues[3] = { tmp_type_s.c_str(), tmp_id_s.c_str(), gid };
	PGresult* sql_res = nullptr;
	try
	{
		sql_res = db_exec_param(db, SQL_DEL_LINK, 3, paramValues, 1);
		logging_res("db_del_link", sql_res);

		if (PQresultStatus(sql_res) == PGRES_TUPLES_OK) result = PQntuples(sql_res);
	}
	catch (const std::exception& ec)
	{
		BOOST_LOG_TRIVIAL(debug) << PQcmdStatus(sql_res);
		BOOST_LOG_TRIVIAL(error) << "db_del_link: " << ec.what();
		result = -1;
	}

	PQclear(sql_res);
	return result;
}

int db_get_face_seq(PGconn* db)
{
	int result = 0;
	PGresult* sql_res = nullptr;
	try
	{
		sql_res = db_exec_param(db, SQL_FACE_TMP_SEQ, 0, nullptr, 1);
		logging_res("db_get_face_seq", sql_res);

		if (PQresultStatus(sql_res) == PGRES_TUPLES_OK && PQntuples(sql_res) > 0)
		{
			int uid_num = PQfnumber(sql_res, "uid");
			result = pg_ntoh64(*(uint64_t*)(PQgetvalue(sql_res, 0, uid_num)));
		}
	}
	catch (const std::exception& ec)
	{
		BOOST_LOG_TRIVIAL(debug) << PQcmdStatus(sql_res);
		BOOST_LOG_TRIVIAL(error) << "db_get_face_seq: " << ec.what();
		result = -1;
	}

	PQclear(sql_res);
	return result;
}

int db_get_finger_seq(PGconn* db)
{
	int result = 0;
	PGresult* sql_res = nullptr;
	try
	{
		sql_res = db_exec_param(db, SQL_FINGER_TMP_SEQ, 0, nullptr, 1);
		logging_res("db_get_finger_seq", sql_res);

		if (PQresultStatus(sql_res) == PGRES_TUPLES_OK && PQntuples(sql_res) > 0)
		{
			int uid_num = PQfnumber(sql_res, "uid");
			result = pg_ntoh64(*(uint64_t*)(PQgetvalue(sql_res, 0, uid_num)));
		}
	}
	catch (const std::exception& ec)
	{
		BOOST_LOG_TRIVIAL(debug) << PQcmdStatus(sql_res);
		BOOST_LOG_TRIVIAL(error) << "db_get_finger_seq: " << ec.what();
		result = -1;
	}

	PQclear(sql_res);
	return result;
}