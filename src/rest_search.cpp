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
                for each (auto el_json in arr)
                {
                    auto json_row = json::value::object();
                    int tmp_type = ABIS_DATA;
                    int tmp_id = 0;

                    int element_type = el_json.at(ELEMENT_TYPE).as_integer();
                    if (element_type == ABIS_FACE_IMAGE)
                    {
                        tmp_type = ABIS_FACE_TEMPLATE;

                        auto element_image = el_json.at(ELEMENT_VALUE).as_string();
                        vector<unsigned char> buf = conversions::from_base64(element_image);

                        float* face_tmp = (float*)malloc(FACE_TEMPLATE_SIZE * sizeof(float));
                        memset(face_tmp, 0, FACE_TEMPLATE_SIZE * sizeof(float));

                        int count = extract_face_template(buf.data(), buf.size(), face_tmp, FACE_TEMPLATE_SIZE * sizeof(float));
                        if (count == 1) tmp_id = db_search_face_template(db, face_tmp);
                        free(face_tmp);

                        if (tmp_id <= 0) continue;
                    }
                    if (element_type == ABIS_FINGER_IMAGE)
                    {
                        tmp_type = ABIS_FINGER_TEMPLATE;

                        auto element_image = el_json.at(ELEMENT_VALUE).as_string();
                        vector<unsigned char> buf = conversions::from_base64(element_image);

                        unsigned char* finger_tmp = (unsigned char*)malloc(FINGER_TEMPLATE_SIZE);
                        int res = get_fingerprint_template(buf.data(), buf.size(), finger_tmp, FINGER_TEMPLATE_SIZE);
                        if (res > 0) tmp_id = db_search_finger_template(db, finger_tmp);
                        free(finger_tmp);

                        if (tmp_id <= 0) continue;
                    }


                    if (element_type == ABIS_FACE_TEMPLATE)
                    {
                        tmp_type = ABIS_FACE_TEMPLATE;
                        void* tmp_arr = json2array(el_json);

                        tmp_id = db_search_face_template(db, tmp_arr);
                        free(tmp_arr);

                        if (tmp_id <= 0) continue;
                    }
                    if (element_type == ABIS_FINGER_TEMPLATE)
                    {
                        tmp_type = ABIS_FINGER_TEMPLATE;
                        void* tmp_arr = json2array(el_json);

                        tmp_id = db_search_finger_template(db, tmp_arr);
                        free(tmp_arr);

                        if (tmp_id <= 0) continue;
                    }

                    char bc_gid[50];
                    int res = db_find_biocard_by_template(db, element_type, tmp_id, bc_gid);
                    if (res <= 0) continue;

                    json_row[ELEMENT_TYPE] = json::value::string(conversions::to_string_t(to_string(tmp_type)));
                    json_row[ELEMENT_UUID] = json::value::string(utility::conversions::to_string_t(bc_gid));
                    json_row[ELEMENT_ID] = json::value::number(tmp_id);
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
