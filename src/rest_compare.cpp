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
                if (arr.size() != 2) throw runtime_error("compare: expect [2]");

                int compare_type = ABIS_DATA;
                for (size_t i = 0; i < 2; i++)
                {
                    int element_type = req_json.at(ELEMENT_TYPE).as_integer();
                    if (element_type == ABIS_FACE_IMAGE) {
                        if (compare_type != ABIS_DATA && compare_type != ABIS_FACE_TEMPLATE)
                            throw runtime_error("compare: mixed types");

                        compare_type = ABIS_FACE_TEMPLATE;

                        auto element_image = arr[i].at(ELEMENT_VALUE).as_string();
                        vector<unsigned char> buf = conversions::from_base64(element_image);

                        void* face_tmp = malloc(FACE_TEMPLATE_SIZE * sizeof(float));
                        tmps.push_back(face_tmp);
                        int count = extract_face_template(buf.data(), buf.size(), face_tmp, FACE_TEMPLATE_SIZE * sizeof(float));

                        if (count != 1) throw runtime_error("compare: get_face_template, return faces <> 1");
                    }

                    if (element_type == ABIS_FINGER_IMAGE)
                    {
                        if (compare_type != ABIS_DATA && compare_type != ABIS_FINGER_TEMPLATE)
                            throw runtime_error("compare: mixed types");

                        compare_type = ABIS_FINGER_TEMPLATE;

                        auto element_image = arr[i].at(ELEMENT_VALUE).as_string();
                        vector<unsigned char> buf = conversions::from_base64(element_image);

                        unsigned char* finger_tmp = (unsigned char*)malloc(FINGER_TEMPLATE_SIZE);
                        tmps.push_back(finger_tmp);
                        get_fingerprint_template(buf.data(), buf.size(), finger_tmp, FINGER_TEMPLATE_SIZE);
                    }

                    if (element_type == ABIS_FACE_TEMPLATE)
                    {
                        if (compare_type != ABIS_DATA && compare_type != ABIS_FACE_TEMPLATE)
                            throw runtime_error("compare: mixed types");

                        compare_type = ABIS_FACE_TEMPLATE;
                        tmps.push_back(json2array(arr[i]));
                    }

                    if (element_type == ABIS_FINGER_TEMPLATE)
                    {
                        if (compare_type != ABIS_DATA && compare_type != ABIS_FINGER_TEMPLATE)
                            throw runtime_error("compare: mixed types");

                        compare_type = ABIS_FINGER_TEMPLATE;
                        tmps.push_back(json2array(arr[i]));
                    }
                }

                float score;
                if (compare_type == ABIS_FACE_TEMPLATE) {
                    score = fvec_eq_dis((const float*)tmps[0], (const float*)tmps[1], FACE_TEMPLATE_SIZE);
                }
                if (compare_type == ABIS_FINGER_TEMPLATE) {
                    score = cmp_fingerprint_template(tmps[0], tmps[1]);
                }

                answer[ELEMENT_VALUE] = json::value::number(score);
                answer[ELEMENT_RESULT] = json::value::boolean(true);
                answer[ELEMENT_TYPE] = json::value::string(conversions::to_string_t(to_string(compare_type)));
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

            for (size_t i = 0; i < 2; i++) free(tmps[i]);
        });

    request.reply(sc, "");
}
