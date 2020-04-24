#include "AbisRest.h"
#include "dbclient.h"
#include "ebsclient.h"
#include "fplibclient.h"

// a helper function to get the number of elements in an array
int getNoEle(char* m)
{
    return ntohl(*(int*)(m + 3 * sizeof(int)));
}

//functions for byte swaping
#define ByteSwap(x)  byteswap((unsigned char*) &x, sizeof(x))
#define byteSwap(x,n)  byteswap((unsigned char*) &x,  n)

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
    int nEle = getNoEle(mem);
    ar = new T[nEle];
    char* start = mem + 5 * sizeof(int);
    int  intSize = sizeof(int);
    for (int i = 0; i < nEle; i++)
    {
        int size = ntohl(*(int*)(start));
        ar[i] = (*(T*)(start + intSize));
        byteSwap(ar[i], size);
        start += size + intSize;
    }
}
void db_get_array(char*& ar, char* mem)
{
    int nEle = getNoEle(mem);
    ar = new char[nEle];
    char* start = mem + 5 * sizeof(int);
    int  intSize = sizeof(int);
    for (int i = 0; i < nEle; i++)
    {
        int size = ntohl(*(int*)(start));
        ar[i] = *(char*)(start + intSize);
        start += size + intSize;
    }
}
void db_get_array(char**& ar, char* mem)
{
    int nEle = getNoEle(mem);
    ar = new char* [nEle];
    char* start = mem + 5 * sizeof(int);
    int  intSize = sizeof(int);
    for (int i = 0; i < nEle; i++)
    {
        int size = ntohl(*(int*)(start));
        ar[i] = new char[size];
        strncpy(ar[i], (char*)(start + intSize), size + 1);
        start += size + intSize;
    }
}

void db_prepare()
{
    PGconn* db = db_open();
    if (PQstatus(db) != CONNECTION_OK) throw runtime_error("Error connection db.");

    // создать таблицы, сиквенсы, подготовленные запросы

    cout << "DB connection success \t";
    cout << PQuser(db) << ":";
    cout << PQpass(db) << "@";
    cout << PQhost(db) << ":";
    cout << PQport(db) << "/";
    cout << PQdb(db) << "";
    cout << endl;

    PQfinish(db);
}

PGconn* db_open()
{
    return PQsetdbLogin(DB_HOST, DB_PORT, "", "", DB_DATABASE, DB_USER, DB_PWD);
}

void db_close(PGconn* db)
{
    PQfinish(db);
}

int db_search_face_template(PGconn* db, const void* tmp_arr)
{
    int result = 0;

    string arr("{");
    for (size_t i = 0; i < FACE_TEMPLATE_SIZE; i++)
    {
        arr.append(to_string(((float*)tmp_arr)[i]));
        if (i != FACE_TEMPLATE_SIZE - 1) arr.append(",");
    }
    arr.append("}");

    int part_size = FACE_TEMPLATE_SIZE / SEARCH_FACE_PARTS;
    string part_size_s = to_string(part_size);

    const char* paramValues[7] = { arr.c_str(),  part_size_s.c_str(), "1",
        DB_DATABASE, SEARCH_FACE_PARAMS, SEARCH_FACE_INDEXS, SEARCH_FACE_VECTORS };

    PGresult* sql_res = nullptr;
    try
    {
        sql_res = PQexecParams(db, SQL_SEARCH_FACE_TMPS, 7, nullptr, paramValues, nullptr, nullptr, 1);

        if (PQresultStatus(sql_res) == PGRES_TUPLES_OK)
        {
            int* arr_ptr;
            char* res_ptr = PQgetvalue(sql_res, 0, 0);
            if (PQgetlength(sql_res, 0, 0) > 0) {
                db_get_array(arr_ptr, res_ptr);
                result = arr_ptr[0];
            }
        }
    }
    catch (const std::exception& ec)
    {
        cout << "search_face_template_id: " << ec.what() << endl;
        result = -1;
    }

    PQclear(sql_res);
    return result;
}


int db_search_finger_template(PGconn* db, const void* tmp_arr)
{
    return 0;
}

int db_find_biocard_by_template(PGconn* db, int tmp_type, int tmp_id, char* gid)
{
    int result = 0;

    string s1 = to_string(tmp_type);
    string s2 = to_string(tmp_id);
    const char* paramValues[2] = { s1.c_str(), s2.c_str() };

    PGresult* sql_res = nullptr;
    try
    {
        sql_res = PQexecParams(db, SQL_LINKS_BY_TMP_ID, 2, nullptr, paramValues, nullptr, nullptr, 0);

        if (PQresultStatus(sql_res) == PGRES_TUPLES_OK)
        {
            int gid_num = PQfnumber(sql_res, "gid");
            char* uuid_ptr = PQgetvalue(sql_res, 0, gid_num);

            strcpy(gid, uuid_ptr);
            result = strlen(gid);
        }
    }
    catch (const std::exception& ec)
    {
        cout << "find_biocard_by_template: " << ec.what() << endl;
        result = -1;
    }

    PQclear(sql_res);
    return result;
}