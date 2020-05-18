#include "restutils.h"
#include "ebsclient.h"


std::wstring s2ws(const std::string& str)
{
    return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
}

std::string ws2s(const std::wstring& wstr)
{
    return utf_to_utf<char>(wstr.c_str(), wstr.c_str() + wstr.size());
}

std::string st2s(const utility::string_t& strt)
{
#ifdef _WIN32
    return ws2s(strt);
#else
    return strt;
#endif
}

void display_json(json::value const& jvalue, std::string const& prefix)
{
    //cout << prefix << jvalue.serialize() << endl;
    cout << prefix << endl;

    if (jvalue.is_object())
    {
        cout << "    {" << endl;
        for (auto const& e : jvalue.as_object())
        {
#ifdef _WIN32
            wcout << "\t{ " << e.first.substr(0, 20);
            wcout << " : \t" << e.second.to_string().substr(0, 50) << "}" << endl;
#else
            cout << "\t{ " << e.first.substr(0, 20);
            cout << " : \t" << e.second.to_string().substr(0, 50) << "}" << endl;
#endif
        }
        cout << "    }" << endl;
    }
    if (jvalue.is_array())
    {
        cout << "  [";
        for (auto const& e : jvalue.as_array())
        {
            display_json(e, "");
        }
        cout << "  ]" << endl;
    }
}

void handle_request(
    http_request request,
    function<void(json::value const&, json::value&)> action)
{
    auto answer = json::value::object();

    http::status_code sc = status_codes::OK;

    try
    {
        request
            .extract_json()
            .then([&answer, &action, &sc](pplx::task<json::value> task)
                {
                    try
                    {
                        auto const& jvalue = task.get();
                        display_json(jvalue, "Request: ");

                        action(jvalue, answer);
                    }
                    catch (http_exception const& ec)
                    {
                        sc = status_codes::BadRequest;
                        //answer[U("exception")] = json::value::string(conversions::to_string_t(ec.what()));
                        cout << ec.what() << endl;
                    }
                })
            .wait();
    }
    catch (const std::exception& ec)
    {
        sc = status_codes::BadRequest;
        //answer[U("exception")] = json::value::string(conversions::to_string_t(ec.what()));

        cout << ec.what() << endl;
    }

    display_json(answer, "Answer: ");
    request.reply(sc, answer);
}

void* json2array(const web::json::value& el)
{
    void* result = nullptr;

    float* tmp = (float*)malloc(ABIS_TEMPLATE_SIZE);
    memset(tmp, 0, ABIS_TEMPLATE_SIZE);

    auto element_tmp = el.at(ELEMENT_VALUE).as_array();
    for (size_t i = 0; i < element_tmp.size(); i++)
    {
        tmp[i] = element_tmp[i].as_double();
    }
    result = tmp;

    return result;
}

int tmp_from_json(json::value el, int& tmp_type, void*& tmp_ptr)
{
    int res = 0;
    int element_type = el.at(ELEMENT_TYPE).as_integer();

    if (element_type == ABIS_FACE_IMAGE)
    {
        auto element_image = el.at(ELEMENT_VALUE).as_string();
        vector<unsigned char> buf = conversions::from_base64(element_image);

        float* face_tmp = (float*)malloc(ABIS_TEMPLATE_SIZE);
        memset(face_tmp, 0, ABIS_TEMPLATE_SIZE);

        tmp_type = ABIS_FACE_TEMPLATE;
        tmp_ptr = face_tmp;
        res = element_type;

        if (extract_face_template(buf.data(), buf.size(), face_tmp, ABIS_TEMPLATE_SIZE) <= 0) res = -1;
    }
    if (element_type == ABIS_FACE_TEMPLATE)
    {
        tmp_type = ABIS_FACE_TEMPLATE;
        tmp_ptr = json2array(el);
        res = element_type;
    }

    if (element_type == ABIS_FINGER_IMAGE)
    {
        auto element_image = el.at(ELEMENT_VALUE).as_string();
        vector<unsigned char> buf = conversions::from_base64(element_image);

        unsigned char* finger_tmp = (unsigned char*)malloc(ABIS_TEMPLATE_SIZE);
        memset(finger_tmp, 0, ABIS_TEMPLATE_SIZE);

        tmp_type = ABIS_FINGER_TEMPLATE;
        tmp_ptr = finger_tmp;
        if (extract_finger_template(buf.data(), buf.size(), finger_tmp, ABIS_TEMPLATE_SIZE, false) <= 0) res = -1;
    }
    if (element_type == ABIS_FINGER_TEMPLATE)
    {
        tmp_type = ABIS_FINGER_TEMPLATE;
        tmp_ptr = json2array(el);
        res = element_type;
    }

    return res;
}