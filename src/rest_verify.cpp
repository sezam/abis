#include "rest_verify.h"
#include "AbisRest.h"
#include "restutils.h"
#include "dbclient.h"
#include "ebsclient.h"
#include "fplibclient.h"

http_listener register_verify(uri url)
{
    http_listener listener(url);
    listener.support(methods::GET, verify_get);

    listener
        .open()
        .then([&listener]() { cout << "starting to listen verify" << endl; })
        .wait();

    return listener;
}

void verify_get(http_request request)
{
    cout << "GET verify " << st2s(request.relative_uri().to_string()) << endl;

    http::status_code sc = status_codes::OK;

    handle_request(
        request,
        [&](json::value const& req_json, json::value& answer)
        {
            PGconn* db = nullptr;
            PGresult* sql_res = nullptr;
            try
            {
                db = db_open();

                auto sp = uri::split_path(request.relative_uri().to_string());
                if (sp.size() != 1) throw runtime_error("GET verify/{uuid} expected.");

                string gids = st2s(sp[0]);
                string_generator gen;
                uuid gid = gen(gids);

                string s1 = to_string(gid);
                const char* paramValues[1] = { s1.c_str() };

                sql_res = PQexecParams(db, SQL_TMP_IDS_BY_BC_GID, 1, nullptr, paramValues, nullptr, nullptr, 1);
                if (PQresultStatus(sql_res) == PGRES_TUPLES_OK && PQntuples(sql_res) > 0)
                {
                    int id_num = PQfnumber(sql_res, "tmp_id");
                    int type_num = PQfnumber(sql_res, "tmp_type");

                    json::array arr = req_json.as_array();
                    auto json_out = json::value::array();
                    for (size_t i = 0; i < arr.size(); i++)
                    {
                        auto json_row = json::value::object();
                        int tmp_id = 0;
                        int json_tmp_type = ABIS_DATA;
                        void* json_tmp_ptr = nullptr;
                        float score = numeric_limits<float>::max();

                        if (tmp_from_json(arr[i], json_tmp_type, json_tmp_ptr) <= 0)
                        {
                            cout << "verify_get: error extract template" << endl;
                            free(json_tmp_ptr);
                            continue;
                        }

                        for (int r = 0; r < PQntuples(sql_res); r++)
                        {
                            int db_tmp_id = ntohl(*(int*)(PQgetvalue(sql_res, r, id_num)));
                            int db_tmp_type = ntohl(*(int*)(PQgetvalue(sql_res, r, type_num)));

                            float cmp = numeric_limits<float>::max();
                            if (db_tmp_type == ABIS_FACE_TEMPLATE && json_tmp_type == ABIS_FACE_TEMPLATE)
                            {
                                void* arr_ptr = malloc(FACE_TEMPLATE_SIZE * sizeof(float));
                                int t = db_face_tmp_by_id(db, db_tmp_id, arr_ptr);
                                if (t > 0) cmp = cmp_face_tmp(json_tmp_ptr, arr_ptr);
                                free(arr_ptr);
                            }
                            if (db_tmp_type == ABIS_FINGER_TEMPLATE && json_tmp_type == ABIS_FINGER_TEMPLATE)
                            {
                                /*
                                void* arr_ptr = malloc(FINGER_TEMPLATE_SIZE);
                                int t = db_finger_tmp_by_id(db, db_tmp_id, arr_ptr);
                                if (t > 0) cmp = cmp_fingerprint_tmp(json_tmp_ptr, arr_ptr);
                                free(arr_ptr);
                                */
                            }
                            if (cmp < score)
                            {
                                score = cmp;
                                tmp_id = db_tmp_id;
                            }
                        }
                        if (tmp_id > 0)
                        {
                            json_row[ELEMENT_ID] = json::value::number(tmp_id);
                            json_row[ELEMENT_VALUE] = json::value::number(score);
                        }
                        free(json_tmp_ptr);

                        json_row[ELEMENT_TYPE] = json::value::number(json_tmp_type);
                        json_out[i] = json_row;
                    }
                    answer[ELEMENT_VALUE] = json_out;
                    answer[ELEMENT_RESULT] = json::value::boolean(true);
                }
                else answer[ELEMENT_RESULT] = json::value::boolean(false);

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
