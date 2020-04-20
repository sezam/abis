#include "compare.h"
#include "AbisRest.h"
#include "restutils.h"
#include "ebsclient.h"
#include "fplibclient.h"
#include "imgutils.h"

http_listener register_compare(uri url)
{
    http_listener listener(url);
    listener.support(methods::GET, compare_get);
    return listener;
}

void compare_get(http_request request)
{
    TRACE(L"\nhandle compare GET\n");

    handle_request(
        request,
        [](json::value const& req_json, json::value& answer)
        {
            void* tmps[2] = { NULL, NULL };
            try
            {
                json::array arr = req_json.as_array();
                if (arr.size() != 2) throw runtime_error("compare: expect array[2]");

                int compare_type = ABIS_DATA;
                for (size_t i = 0; i < 2; i++)
                {
                    int element_type = ABIS_DATA;

                    json::value el_type = arr[i].at(ELEMENT_TYPE);
                    if (el_type.is_integer()) element_type = el_type.as_integer();
                    if (el_type.is_string()) element_type = stoi(el_type.as_string());

                    if (element_type == ABIS_FACE_IMAGE) {
                        if (compare_type != ABIS_DATA && compare_type != ABIS_FACE_TEMPLATE) {
                            throw runtime_error("compare: mixed types");
                        }

                        compare_type = ABIS_FACE_TEMPLATE;

                        auto element_image = arr[i].at(ELEMENT_VALUE).as_string();
                        vector<unsigned char> buf = conversions::from_base64(element_image);

                        void* face_tmp = malloc(FACE_TEMPLATE_SIZE * sizeof(float));
                        tmps[i] = face_tmp;
                        int count = 0;
                        try
                        {
                            count = get_face_template(buf.data(), buf.size(), face_tmp, FACE_TEMPLATE_SIZE * sizeof(float));
                        }
                        catch (const std::exception&) {
                            throw runtime_error("compare: get_face_template");
                        }

                        if (count != 1)
                        {
                            throw runtime_error("compare: get_face_template, return faces <> 1");
                        }
                    }

                    if (element_type == ABIS_FINGER_IMAGE)
                    {
                        if (compare_type != ABIS_DATA && compare_type != ABIS_FINGER_TEMPLATE) {
                            throw runtime_error("compare: mixed types");
                        }

                        compare_type = ABIS_FINGER_TEMPLATE;

                        auto element_image = arr[i].at(ELEMENT_VALUE).as_string();
                        vector<unsigned char> buf = conversions::from_base64(element_image);

                        unsigned char* finger_tmp = (unsigned char*)malloc(FINGER_TEMPLATE_SIZE);
                        tmps[i] = finger_tmp;
                        try
                        {
                            get_fingerprint_template(buf.data(), buf.size(), finger_tmp, FINGER_TEMPLATE_SIZE);

                        }
                        catch (const std::exception&)
                        {
                            throw runtime_error("compare: get_fingerprint_template");
                        }
                    }

                    if (element_type == ABIS_FACE_TEMPLATE) {
                        if (compare_type != ABIS_DATA && compare_type != ABIS_FACE_TEMPLATE) {
                            throw runtime_error("compare: mixed types");
                        }

                        compare_type = ABIS_FACE_TEMPLATE;

                        float* face_tmp = (float*)malloc(FACE_TEMPLATE_SIZE * sizeof(float));
                        tmps[i] = face_tmp;

                        auto element_tmp = arr[i].at(ELEMENT_VALUE).as_array();
                        for (size_t i = 0; i < element_tmp.size(); i++)
                        {
                            face_tmp[i] = element_tmp[i].as_double();
                        }
                    }

                    if (element_type == ABIS_FINGER_TEMPLATE) {
                        if (compare_type != ABIS_DATA && compare_type != ABIS_FINGER_TEMPLATE) {
                            throw runtime_error("compare: mixed types");
                        }

                        compare_type = ABIS_FINGER_TEMPLATE;

                        unsigned char* finger_tmp = (unsigned char*)malloc(FINGER_TEMPLATE_SIZE);
                        tmps[i] = finger_tmp;

                        auto element_tmp = arr[i].at(ELEMENT_VALUE).as_array();
                        //for (size_t i = 0; i < FINGER_TEMPLATE_SIZE; i++)
                        for (size_t i = 0; i < element_tmp.size(); i++)
                        {
                            finger_tmp[i] = element_tmp[i].as_integer();
                        }
                    }
                }

                float score;
                if (compare_type == ABIS_FACE_TEMPLATE) {
                    score = fvec_eq_dis((const float*)tmps[0], (const float*)tmps[1], FACE_TEMPLATE_SIZE);
                }
                if (compare_type == ABIS_FINGER_TEMPLATE) {
                    score = cmp_fingerprint_template(tmps[0], tmps[1]);
                }

                for (size_t i = 0; i < 2; i++) free(tmps[i]);

                answer[ELEMENT_VALUE] = json::value::number(score);
                answer[ELEMENT_RESULT] = json::value::boolean(true);
                answer[ELEMENT_TYPE] = json::value::string(conversions::to_string_t(to_string(compare_type)));
            }
            catch (const boost::system::error_code& ec)
            {
                answer[ELEMENT_RESULT] = json::value::boolean(false);
                std::string val = STD_TO_UTF(ec.message());
                answer[ELEMENT_ERROR] = json::value::string(conversions::to_string_t(val));

                cout << "Exception: " << ec.message() << endl;
            }
            catch (const std::exception& ec)
            {
                answer[ELEMENT_RESULT] = json::value::boolean(false);
                std::string val = STD_TO_UTF(ec.what());
                answer[ELEMENT_ERROR] = json::value::string(conversions::to_string_t(val));

                cout << "Exception: " << ec.what() << endl;
            }
        });

    request.reply(status_codes::OK, "");
}
