#include "rest_biocard.h"
#include "AbisRest.h"
#include "restutils.h"
#include "dbclient.h"

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
            PGconn* db = NULL;
            try
            {
                auto sp = uri::split_path(request.relative_uri().to_string());
                string_generator gen;
                uuid gid = gen(st2s(sp[0]));

                if (sp.size() != 1) throw runtime_error("GET biocard/{uuid} expected.");

                db = DBstart();
                DBassert(db, CONNECTION_OK, "Error connection db.");

                string uu = to_string(gid);

                const char* paramValues[1] = { uu.c_str() };
                PGresult* sql_res = DBexec2(db, SQL_SELECT_BC_TMPS, paramValues);

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
                PQclear(sql_res);
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
            DBfinish(db);
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

            DBfinish(db);
        });

    request.reply(sc, "");
}

void biocard_patch(http_request request)
{
    TRACE(L"PATCH biocard\n");

    request.reply(status_codes::OK, "");
}
