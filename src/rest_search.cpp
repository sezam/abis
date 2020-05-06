#include "rest_search.h"
#include "AbisRest.h"
#include "restutils.h"
#include "dbclient.h"
#include "ebsclient.h"
#include "fplibclient.h"

http_listener register_search(uri url)
{
    http_listener listener(url);
    listener.support(methods::GET, search_get);

    listener
        .open()
        .then([&listener]() { cout << "starting to listen search" << endl; })
        .wait();

    return listener;
}

void search_get(http_request request)
{
    cout << "GET search " << st2s(request.relative_uri().to_string()) << endl;

    http::status_code sc = status_codes::OK;

    handle_request(
        request,
        [&](json::value const& req_json, json::value& answer)
        {
            PGconn* db = nullptr;
            try
            {
                db = db_open();

                json::array arr = req_json.as_array();
                auto json_out = json::value::array();
                int rows = 0;
                for (size_t i = 0; i < arr.size(); i++)
                {
                    auto json_row = json::value::object();
                    int json_tmp_type = ABIS_DATA;
                    void* json_tmp_ptr = nullptr;
                    int tmp_id = 0;
                    float score = 0;
                    int step = 0;

                    if (tmp_from_json(arr[i], json_tmp_type, json_tmp_ptr) <= 0)
                    {
                        cout << "verify_get: error extract template" << endl;
                        free(json_tmp_ptr);
                        continue;
                    }

                    if (json_tmp_type == ABIS_FACE_TEMPLATE)
                    {
                        step = tmp_id = db_search_face_tmp(db, json_tmp_ptr);
                        if (step > 0)
                        {
                            int tmp_size = FACE_TEMPLATE_SIZE * sizeof(float);

                            void* face_tmp = malloc(tmp_size);
                            memset(face_tmp, 0, tmp_size);

                            step = db_face_tmp_by_id(db, tmp_id, face_tmp);
                            if (step > 0) score = cmp_face_tmp(json_tmp_ptr, face_tmp);
                            free(face_tmp);
                        }
                    }
                    if (json_tmp_type == ABIS_FINGER_TEMPLATE)
                    {
                        step = tmp_id = db_search_finger_tmp(db, json_tmp_ptr);
                        if (tmp_id > 0)
                        {
                            void* finger_tmp = malloc(FINGER_TEMPLATE_SIZE);
                            memset(finger_tmp, 0, FINGER_TEMPLATE_SIZE);
                            // db_finger_search...
                            if (step > 0) score = cmp_fingerprint_tmp(json_tmp_ptr, finger_tmp);
                            free(finger_tmp);
                        }
                    }

                    char bc_gid[50];
                    if (step > 0) json_row[ELEMENT_ID] = json::value::number(tmp_id);
                    if (step > 0) step = db_card_id_by_tmp_id(db, json_tmp_type, tmp_id, bc_gid);
                    if (step > 0)
                    {
                        json_row[ELEMENT_UUID] = json::value::string(conversions::to_string_t(bc_gid));                        
                        json_row[ELEMENT_VALUE] = json::value::number(score);
                    }
                    json_row[ELEMENT_TYPE] = json::value::number(json_tmp_type);
                    json_out[rows++] = json_row;
                    free(json_tmp_ptr);
                }
                answer[ELEMENT_VALUE] = json_out;
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
