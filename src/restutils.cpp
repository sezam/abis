#include "restutils.h"


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