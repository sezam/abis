#include "AbisRest.h"
#include "rest_extract.h"
#include "restutils.h"
#include "ebsclient.h"

http_listener register_extract(web::uri url)
{
    http_listener listener(url);
    listener.support(methods::GET, extract_get);

    listener
        .open()
        .then([&listener]() { cout << "starting to listen extact" << endl; })
        .wait();

    return listener;
}

void extract_get(http_request request)
{
    TRACE(L"GET extract\n");

    http::status_code sc = status_codes::OK;

    handle_request(
        request,
        [&](json::value const& req_json, json::value& answer)
        {
            try
            {
                auto element_image = req_json.at(ELEMENT_VALUE).as_string();
                vector<unsigned char> buf = conversions::from_base64(element_image);

                int template_type = ABIS_DATA;
                int element_type = req_json.at(ELEMENT_TYPE).as_integer();
                if (element_type == ABIS_FACE_IMAGE)
                {
                    template_type = ABIS_FACE_TEMPLATE;
                    float* face_tmp = (float*)malloc(ABIS_TEMPLATE_SIZE);
                    memset(face_tmp, 0, ABIS_TEMPLATE_SIZE);

                    int count = extract_face_template(buf.data(), buf.size(), face_tmp, ABIS_TEMPLATE_SIZE);
                    if (count == 1) 
                    {
                        for (size_t i = 0; i < ABIS_TEMPLATE_LEN; i++)
                        {
                            answer[ELEMENT_VALUE][i] = json::value::number(face_tmp[i]);
                        }
                    }
                    free(face_tmp);
                }

                if (element_type == ABIS_FINGER_IMAGE)
                {
                    template_type = ABIS_FINGER_TEMPLATE;
                    unsigned char* finger_tmp = (unsigned char*)malloc(ABIS_TEMPLATE_SIZE);
                    memset(finger_tmp, 0, ABIS_TEMPLATE_SIZE);

                    int res = extract_finger_template(buf.data(), buf.size(), finger_tmp, ABIS_TEMPLATE_SIZE);
                    if (res > 0)
                    {
                        for (size_t i = 0; i < ABIS_TEMPLATE_LEN; i++)
                        {
                            answer[ELEMENT_VALUE][i] = json::value::number(finger_tmp[i]);
                        }
                    }
                    free(finger_tmp);
                }

                answer[ELEMENT_RESULT] = json::value::boolean(true);
                answer[ELEMENT_TYPE] = json::value::number(template_type);
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
        });

    request.reply(sc, "");
}
