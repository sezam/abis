#include "rest_biocard.h"
#include "AbisRest.h"
#include "restutils.h"
#include "dbclient.h"
#include "restutils.h"

http_listener register_biocard(uri url)
{
    http_listener listener(url);
    listener.support(methods::GET, biocard_get);
    listener.support(methods::POST, biocard_post);
    listener.support(methods::PUT, biocard_put);
    listener.support(methods::PATCH, biocard_patch);

    listener
        .open()
        .then([&listener]() { cout << "starting to listen biocard" << endl; })
        .wait();

    return listener;
}

void biocard_get(http_request request)
{
    TRACE(L"GET biocard\n");

    http::status_code sc = status_codes::OK;

    handle_request(
        request,
        [&](json::value const& req_json, json::value& answer)
        {
            PGconn* db = nullptr;
            PGresult* sql_res = nullptr;
            try
            {
                auto sp = uri::split_path(request.relative_uri().to_string());
                if (sp.size() != 1) throw runtime_error("GET biocard/{uuid} expected.");

                string_generator gen;
                uuid gid = gen(st2s(sp[0]));

                db = db_open();

                string s1 = to_string(gid);
                const char* paramValues[1] = { s1.c_str() };

                sql_res = PQexecParams(db, SQL_LINKS_BY_BC_GID, 1, nullptr, paramValues, nullptr, nullptr, 1);
                if (PQresultStatus(sql_res) == PGRES_TUPLES_OK)
                {
                    int type_num = PQfnumber(sql_res, "tmp_type");
                    int id_num = PQfnumber(sql_res, "tmp_id");

                    auto json_out = json::value::array();
                    for (size_t i = 0; i < PQntuples(sql_res); i++)
                    {
                        auto json_row = json::value::object();

                        char* ptr = PQgetvalue(sql_res, i, type_num);
                        int tmp = ntohl(*((uint32_t*)ptr));
                        json_row[ELEMENT_TYPE] = json::value::number(tmp);

                        ptr = PQgetvalue(sql_res, i, id_num);
                        tmp = ntohl(*((uint32_t*)ptr));
                        json_row[ELEMENT_ID] = json::value::number(tmp);
                        json_out[i] = json_row;
                    }
                    answer[ELEMENT_VALUE] = json_out;
                    answer[ELEMENT_RESULT] = json::value::boolean(true);
                }
            }
            catch (const boost::system::error_code& ec)
            {
                sc = status_codes::BadRequest;
                answer[ELEMENT_RESULT] = json::value::boolean(false);
                cout << "Exception: " << ec.message() << endl;
            }
            catch (const std::exception& ec)
            {
                sc = status_codes::BadRequest;
                answer[ELEMENT_RESULT] = json::value::boolean(false);
                cout << "Exception: " << ec.what() << endl;
            }
            PQclear(sql_res);
            db_close(db);
        });

    request.reply(sc, "");
}

void biocard_post(http_request request)
{
    TRACE(L"POST biocard\n");

    request.reply(status_codes::OK, "");
}

void biocard_put(http_request request)
{
    TRACE(L"PUT biocard\n");

    http::status_code sc = status_codes::OK;

    handle_request(
        request,
        [&](json::value const& req_json, json::value& answer)
        {
            PGconn* db = NULL;
            try
            {
                uuid gid;

                auto sp = uri::split_path(request.relative_uri().to_string());
                if (sp.size() == 1)
                {
                    string_generator gen;
                    gid = gen(st2s(sp[0]));
                }


            }
            catch (const boost::system::error_code& ec)
            {
                sc = status_codes::BadRequest;
                answer[ELEMENT_RESULT] = json::value::boolean(false);
                cout << "Exception: " << ec.message() << endl;
            }
            catch (const std::exception& ec)
            {
                sc = status_codes::BadRequest;
                answer[ELEMENT_RESULT] = json::value::boolean(false);
                cout << "Exception: " << ec.what() << endl;
            }

            db_close(db);
        });

    request.reply(sc, "");
}

void biocard_patch(http_request request)
{
    TRACE(L"PATCH biocard\n");

    request.reply(status_codes::OK, "");
}
