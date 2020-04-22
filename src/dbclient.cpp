#include "AbisRest.h"
#include "dbclient.h"

void DBprepare()
{
    PGconn* db = DBstart();
    DBassert(db, CONNECTION_OK, "Error connection db.");

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

PGconn* DBstart()
{
    return PQsetdbLogin(DB_HOST, DB_PORT, "", "", DB_DATABASE, DB_USER, DB_PWD);
}

void DBfinish(PGconn* db)
{
    PQfinish(db);
}

bool DBstatus(PGconn* db, const ConnStatusType status)
{
    return (PQstatus(db) == status);
}

void DBassert(PGconn* db, const ConnStatusType status, const string& error)
{
    if (!DBstatus(db, status))
    {
        cout << "Check PQstatus() fail:  " << PQstatus(db) << " " << error << endl;
        throw runtime_error(error);
    }
}

bool DBstatus(PGresult* res, const ExecStatusType status)
{
    return PQresultStatus(res) == status;
}

void DBassert(PGresult* res, const ExecStatusType status, const string& error)
{
    if (!DBstatus(res, status))
    {
        cout << "Check PQresultStatus() fail:  " << PQresultStatus(res) << " " << error << endl;

        PQclear(res);
        throw runtime_error(error);
    }
}

void DBassert(PGresult* res, const ExecStatusType status, const string& error, const string& info)
{
    if (!DBstatus(res, status))
    {
        cout << "Check PQresultStatus() fail:  " << PQresultStatus(res) << " " << error << " " << info << endl;

        PQclear(res);
        throw runtime_error(error + info);
    }
}

PGresult* DBexec(PGconn* db, const char* query, const char* params[])
{
    return PQexecParams(db, SQL_SELECT_BC_TMPS, 1, NULL, params, NULL, NULL, 1);
}

void DBexec0(PGconn* db, char* const query, const char* params[])
{
    PGresult* sql_res = DBexec(db, query, params);
    DBassert(sql_res, PGRES_COMMAND_OK, "Error exec query: ", query);
}

PGresult* DBexec2(PGconn* db, const char* query, const char* params[])
{
    PGresult* sql_res = DBexec(db, query, params);
    DBassert(sql_res, PGRES_TUPLES_OK, "Error exec query: ", query);
    return sql_res;
}
