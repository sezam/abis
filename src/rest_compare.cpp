#include "rest_compare.h"
#include "AbisRest.h"
#include "restutils.h"
#include "ebsclient.h"
#include "fplibclient.h"
#include "imgutils.h"

http_listener register_compare(uri url)
{
    http_listener listener(url);
    listener.support(methods::GET, compare_get);

    listener
        .open()
        .then([&listener]() { cout << "starting to listen compare" << endl; })
        .wait();

    return listener;
}

void compare_get(http_request request)
{
    TRACE(L"GET compare\n");

    http::status_code sc = status_codes::OK;

    handle_request(
        request,
        [&](json::value const& req_json, json::value& answer)
        {
            vector<void*> tmps;
            try
            {
                json::array arr = req_json.as_array();
                if (arr.size() != 2) throw runtime_error("compare: expected array[2]");

                int compare_type = ABIS_DATA;
                for (size_t i = 0; i < 2; i++)
                {
                    int json_tmp_type = ABIS_DATA;
                    void* json_tmp_ptr = nullptr;

                    if (tmp_from_json(arr[i], json_tmp_type, json_tmp_ptr) <= 0)
                    {
                        cout << "compare_get: error extract template" << endl;
                        free(json_tmp_ptr);
                        throw runtime_error("compare: error extract template");
                    }
                    tmps.push_back(json_tmp_ptr);

                    if (compare_type == ABIS_DATA) compare_type = json_tmp_type;
                    if (compare_type != json_tmp_type) throw runtime_error("compare: mixed types");
                }

                float score = 0;
                if (compare_type == ABIS_FACE_TEMPLATE) {
                    score = cmp_face_tmp(tmps[0], tmps[1]);
                }
                if (compare_type == ABIS_FINGER_TEMPLATE) {
                    score = cmp_fingerprint_tmp(tmps[0], tmps[1]);
                }

                answer[ELEMENT_VALUE] = json::value::number(score);
                answer[ELEMENT_RESULT] = json::value::boolean(true);
                answer[ELEMENT_TYPE] = json::value::number(compare_type);
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

            for (size_t i = 0; i < tmps.size(); i++) free(tmps[i]);
        });

    request.reply(sc, "");
}
