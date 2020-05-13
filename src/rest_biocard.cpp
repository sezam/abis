#include "rest_biocard.h"
#include "AbisRest.h"
#include "restutils.h"
#include "dbclient.h"
#include "ebsclient.h"

http_listener register_biocard(uri url)
{
    http_listener listener(url);
    listener.support(methods::GET, biocard_get);
    listener.support(methods::PUT, biocard_put);
    listener.support(methods::DEL, biocard_del);

    listener
        .open()
        .then([&listener]() { cout << "starting to listen biocard" << endl; })
        .wait();

    return listener;
}

void biocard_get(http_request request)
{
    cout << "GET biocard" << st2s(request.relative_uri().to_string()) << endl;

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

                sql_res = db_exec_param(db, SQL_TMP_IDS_BY_BC_GID, 1, paramValues, 1);
                if (PQresultStatus(sql_res) == PGRES_TUPLES_OK)
                {
                    int type_num = PQfnumber(sql_res, "tmp_type");
                    int id_num = PQfnumber(sql_res, "tmp_id");

                    auto json_out = json::value::array();
                    for (int i = 0; i < PQntuples(sql_res); i++)
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
                else
                {
                    answer[ELEMENT_ERROR] = json::value::string(conversions::to_string_t(PQerrorMessage(db)));
                    cout << "biocard_get: " << PQerrorMessage(db) << endl;
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

void biocard_put(http_request request)
{
    cout << "PUT biocard " << st2s(request.relative_uri().to_string()) << endl;

    http::status_code sc = status_codes::OK;

    handle_request(
        request,
        [&](json::value const& req_json, json::value& answer)
        {
            PGconn* db = nullptr;
            try
            {
                db = db_open();
                db_tx_begin(db);

                uuid gid;
                int bc_id = 0;

                auto sp = uri::split_path(request.relative_uri().to_string());
                if (sp.size() == 0)
                {
                    basic_random_generator<boost::mt19937> gen;
                    gid = gen();

                    bc_id = db_add_bc(db, to_string(gid).c_str(), "");
                    if (bc_id <= 0) throw runtime_error("biocard_put: biocard not added.");
                }
                if (sp.size() > 0)
                {
                    string_generator gen;
                    gid = gen(st2s(sp[0]));

                    bc_id = db_card_id_by_gid(db, to_string(gid).c_str());
                    if (bc_id <= 0) throw runtime_error("biocard_put: biocard not found.");
                }

                int inserted = 0;
                auto json_out = json::value::array();
                auto arr = req_json.at(ELEMENT_VALUE).as_array();
                for (size_t i = 0; i < arr.size(); i++)
                {
                    int tmp_type = ABIS_DATA;
                    int tmp_id = 0;
                    void* tmp_arr = nullptr;
                    auto json_row = json::value::object();
                    int step = 0;

                    if (tmp_from_json(arr[i], tmp_type, tmp_arr) <= 0)
                    {
                        cout << "biocard_put: error extract template" << endl;
                        free(tmp_arr);
                        continue;
                    }

                    if (tmp_type == ABIS_FACE_TEMPLATE)
                    {
                        db_sp_begin(db, "face_template");
                        step = tmp_id = db_search_face_tmp(db, tmp_arr);
                        if (step > 0)
                        {
                            float score = 0;

                            void* face_tmp = malloc(ABIS_TEMPLATE_SIZE);
                            memset(face_tmp, 0, ABIS_TEMPLATE_SIZE);

                            step = db_face_tmp_by_id(db, tmp_id, face_tmp);
                            if (step > 0) score = cmp_face_tmp(tmp_arr, face_tmp);
                            free(face_tmp);

                            if (score >= ABIS_FACE_THRESHOLD)
                            {
                                char bc_gid[50];
                                step = (int) db_card_id_by_tmp_id(db, tmp_type, tmp_id, bc_gid) == 0;
                            }
                            else
                            {
                                step = tmp_id = db_get_face_seq(db);
                                if (step > 0) step = db_insert_face_tmp(db, tmp_arr, tmp_id);
                            }
                        }

                        if (step > 0) step = db_add_link(db, ABIS_FACE_TEMPLATE, tmp_id, bc_id);
                        if (step > 0) inserted++;
                        if (step <= 0) db_sp_rollback(db, "face_template");
                    }
                    if (tmp_type == ABIS_FINGER_TEMPLATE)
                    {
                        db_sp_begin(db, "finger_template");
                        step = tmp_id = db_search_finger_tmp(db, tmp_arr);
                        if (step > 0)
                        {
                            float score = 0;

                            void* face_tmp = malloc(ABIS_TEMPLATE_SIZE);
                            memset(face_tmp, 0, ABIS_TEMPLATE_SIZE);

                            step = db_finger_tmp_by_id(db, tmp_id, face_tmp);
                            if (step > 0) score = cmp_finger_tmp(tmp_arr, face_tmp);
                            free(face_tmp);

                            if (score >= ABIS_FINGER_THRESHOLD)
                            {
                                char bc_gid[50];
                                step = (int)db_card_id_by_tmp_id(db, tmp_type, tmp_id, bc_gid) == 0;
                            }
                            else
                            {
                                step = tmp_id = db_get_finger_seq(db);
                                if (step > 0) step = db_insert_finger_tmp(db, tmp_arr, tmp_id);
                            }
                        }

                        if (step > 0) step = db_add_link(db, ABIS_FINGER_TEMPLATE, tmp_id, bc_id);
                        if (step > 0) inserted++;
                        if (step <= 0) db_sp_rollback(db, "finger_template");
                    }
                    free(tmp_arr);

                    if (step > 0) json_row[ELEMENT_ID] = json::value::number(tmp_id);
                    json_row[ELEMENT_TYPE] = json::value::number(tmp_type);
                    json_row[ELEMENT_RESULT] = json::value::boolean(step > 0);

                    json_out[i] = json_row;
                }
                if (inserted > 0)
                {
                    db_tx_commit(db);
                    answer[ELEMENT_UUID] = json::value::string(conversions::to_string_t(to_string(gid)));
                }
                else db_tx_rollback(db);
                answer[ELEMENT_VALUE] = json_out;
                answer[ELEMENT_RESULT] = json::value::boolean(true);
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

void biocard_del(http_request request)
{
    cout << "DELETE biocard" << st2s(request.relative_uri().to_string()) << endl;

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
                if (sp.size() != 1) throw runtime_error("DELETE biocard/{uuid} expected.");

                string_generator gen;
                uuid gid = gen(st2s(sp[0]));

                db = db_open();

                auto json_out = json::value::array();
                auto arr = req_json.as_array();
                for (size_t i = 0; i < arr.size(); i++)
                {
                    int tmp_type = arr[i].at(ELEMENT_TYPE).as_integer();
                    int tmp_id = arr[i].at(ELEMENT_ID).as_integer();
                    int del_res = db_del_link(db, tmp_type, tmp_id, to_string(gid).c_str());

                    auto json_row = arr[i];
                    json_row[ELEMENT_RESULT] = json::value::boolean(del_res > 0);
                    json_out[i] = json_row;
                }

                answer[ELEMENT_VALUE] = json_out;
                answer[ELEMENT_RESULT] = json::value::boolean(true);
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
