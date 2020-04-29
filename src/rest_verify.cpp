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
    TRACE(L"GET verify\n");

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
                if (PQresultStatus(sql_res) == PGRES_TUPLES_OK)
                {
                    int id_num = PQfnumber(sql_res, "tmp_id");
                    int type_num = PQfnumber(sql_res, "tmp_type");

                    json::array arr = req_json.as_array();
                    auto json_out = json::value::array();
                    for (size_t i = 0; i < arr.size(); i++)
                    {
                        auto json_row = json::value::object();
                        int tmp_type = ABIS_DATA;
                        int tmp_id = 0;
                        float score = numeric_limits<float>::max();
                        void* tmp_ptr = nullptr;

                        int element_type = arr[i].at(ELEMENT_TYPE).as_integer();
                        if (element_type == ABIS_FACE_IMAGE)
                        {
                            tmp_type = ABIS_FACE_TEMPLATE;
                            int tmp_size = FACE_TEMPLATE_SIZE * sizeof(float);

                            auto element_image = arr[i].at(ELEMENT_VALUE).as_string();
                            vector<unsigned char> buf = conversions::from_base64(element_image);

                            float* face_tmp = (float*)malloc(tmp_size);
                            memset(face_tmp, 0, tmp_size);

                            int count = extract_face_template(buf.data(), buf.size(), face_tmp, tmp_size);
                            if (count != 1) tmp_ptr = face_tmp;
                        }
                        if (element_type == ABIS_FINGER_IMAGE)
                        {
                            tmp_type = ABIS_FINGER_TEMPLATE;

                            auto element_image = arr[i].at(ELEMENT_VALUE).as_string();
                            vector<unsigned char> buf = conversions::from_base64(element_image);

                            unsigned char* finger_tmp = (unsigned char*)malloc(FINGER_TEMPLATE_SIZE);
                            int res = get_fingerprint_template(buf.data(), buf.size(), finger_tmp, FINGER_TEMPLATE_SIZE);
                            if (res <= 0) tmp_ptr = finger_tmp;
                        }

                        if (element_type == ABIS_FACE_TEMPLATE)
                        {
                            tmp_type = ABIS_FACE_TEMPLATE;
                            tmp_ptr = json2array(arr[i]);
                        }
                        if (element_type == ABIS_FINGER_TEMPLATE)
                        {
                            tmp_type = ABIS_FINGER_TEMPLATE;
                            tmp_ptr = json2array(arr[i]);
                        }

                        if (tmp_ptr != nullptr)
                        {
                            for (int r = 0; r < PQntuples(sql_res); r++)
                            {
                                int id_tmp = ntohl(*(int*)(PQgetvalue(sql_res, r, id_num)));
                                int type_tmp = ntohl(*(int*)(PQgetvalue(sql_res, r, type_num)));

                                float cmp = numeric_limits<float>::max();
                                if (type_tmp == ABIS_FACE_TEMPLATE && tmp_type == ABIS_FACE_TEMPLATE)
                                {
                                    void* arr_ptr = malloc(FACE_TEMPLATE_SIZE * sizeof(float));
                                    int t = db_face_tmp_by_id(db, id_tmp, arr_ptr);
                                    if (t > 0) cmp = cmp_face_tmp(tmp_ptr, arr_ptr);
                                    free(arr_ptr);
                                }
                                if (type_tmp == ABIS_FINGER_TEMPLATE && tmp_type == ABIS_FINGER_TEMPLATE)
                                {
                                    void* arr_ptr = malloc(FINGER_TEMPLATE_SIZE);
                                    int t = db_face_tmp_by_id(db, id_tmp, arr_ptr);//db_finger...
                                    if (t > 0) cmp = cmp_fingerprint_tmp(tmp_ptr, arr_ptr);
                                    free(arr_ptr);
                                }
                                if (cmp < score)
                                {
                                    score = cmp;
                                    tmp_id = id_tmp;
                                }
                            }
                            free(tmp_ptr);
                        }
                        json_row[ELEMENT_TYPE] = json::value::number(tmp_type);
                        if (tmp_id > 0)json_row[ELEMENT_ID] = json::value::number(tmp_id);
                        json_row[ELEMENT_VALUE] = json::value::number(score);
                        json_out[i] = json_row;
                    }
                    answer[ELEMENT_VALUE] = json_out;
                }
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
