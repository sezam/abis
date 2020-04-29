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
    TRACE(L"GET search\n");

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
                for (auto el_json : arr)
                {
                    auto json_row = json::value::object();
                    int tmp_type = ABIS_DATA;
                    int tmp_id = 0;
                    float score = 0;

                    int element_type = el_json.at(ELEMENT_TYPE).as_integer();
                    if (element_type == ABIS_FACE_IMAGE)
                    {
                        tmp_type = ABIS_FACE_TEMPLATE;
                        int tmp_size = FACE_TEMPLATE_SIZE * sizeof(float);

                        auto element_image = el_json.at(ELEMENT_VALUE).as_string();
                        vector<unsigned char> buf = conversions::from_base64(element_image);

                        float* face_tmp = (float*)malloc(tmp_size);
                        memset(face_tmp, 0, tmp_size);

                        int count = extract_face_template(buf.data(), buf.size(), face_tmp, tmp_size);
                        if (count == 1) tmp_id = db_search_face_tmp(db, face_tmp);
                        if (tmp_id > 0)
                        {
                            void* face_tmp2 = malloc(tmp_size);
                            memset(face_tmp2, 0, tmp_size);
                            int fl = db_face_tmp_by_id(db, tmp_id, face_tmp2);
                            if (fl == tmp_size) score = cmp_face_tmp(face_tmp, face_tmp2);
                            free(face_tmp2);
                        }
                        free(face_tmp);

                        if (tmp_id <= 0) continue;
                    }
                    if (element_type == ABIS_FINGER_IMAGE)
                    {
                        tmp_type = ABIS_FINGER_TEMPLATE;

                        auto element_image = el_json.at(ELEMENT_VALUE).as_string();
                        vector<unsigned char> buf = conversions::from_base64(element_image);

                        void* finger_tmp = malloc(FINGER_TEMPLATE_SIZE);
                        memset(finger_tmp, 0, FINGER_TEMPLATE_SIZE);

                        int res = get_fingerprint_template(buf.data(), buf.size(), (unsigned char*)finger_tmp, FINGER_TEMPLATE_SIZE);
                        if (res > 0) tmp_id = db_search_finger_tmp(db, finger_tmp);
                        if (tmp_id > 0)
                        {
                            void* finger_tmp2 = malloc(FINGER_TEMPLATE_SIZE);
                            memset(finger_tmp2, 0, FINGER_TEMPLATE_SIZE);
                            int fl = db_face_tmp_by_id(db, tmp_id, finger_tmp2); // fb_finger...
                            if (fl == FINGER_TEMPLATE_SIZE) score = cmp_fingerprint_tmp(finger_tmp, finger_tmp2);
                            free(finger_tmp2);
                        }
                        free(finger_tmp);

                        if (tmp_id <= 0) continue;
                    }


                    if (element_type == ABIS_FACE_TEMPLATE)
                    {
                        tmp_type = ABIS_FACE_TEMPLATE;
                        void* tmp_arr = json2array(el_json);
                        int tmp_size = FACE_TEMPLATE_SIZE * sizeof(float);

                        tmp_id = db_search_face_tmp(db, tmp_arr);
                        if (tmp_id > 0)
                        {
                            void* face_tmp2 = malloc(tmp_size);
                            memset(face_tmp2, 0, tmp_size);
                            int fl = db_face_tmp_by_id(db, tmp_id, face_tmp2);
                            if (fl == tmp_size) score = cmp_face_tmp(tmp_arr, face_tmp2);
                            free(face_tmp2);
                        }
                        free(tmp_arr);

                        if (tmp_id <= 0) continue;
                    }
                    if (element_type == ABIS_FINGER_TEMPLATE)
                    {
                        tmp_type = ABIS_FINGER_TEMPLATE;
                        void* tmp_arr = json2array(el_json);

                        tmp_id = db_search_finger_tmp(db, tmp_arr);
                        if (tmp_id > 0)
                        {
                            void* finger_tmp2 = malloc(FINGER_TEMPLATE_SIZE);
                            memset(finger_tmp2, 0, FINGER_TEMPLATE_SIZE);
                            int fl = db_face_tmp_by_id(db, tmp_id, finger_tmp2); // db_finger...
                            if (fl == FINGER_TEMPLATE_SIZE) score = cmp_fingerprint_tmp(tmp_arr, finger_tmp2);
                            free(finger_tmp2);
                        }
                        free(tmp_arr);

                        if (tmp_id <= 0) continue;
                    }

                    char bc_gid[50];
                    int res = db_card_id_by_tmp_id(db, element_type, tmp_id, bc_gid);
                    if (res <= 0) continue;

                    json_row[ELEMENT_TYPE] = json::value::number(tmp_type);
                    json_row[ELEMENT_UUID] = json::value::string(conversions::to_string_t(bc_gid));
                    json_row[ELEMENT_ID] = json::value::number(tmp_id);
                    json_row[ELEMENT_VALUE] = json::value::number(score);
                    json_out[rows++] = json_row;
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
