#include "AbisRest.h"
#include "restutils.h"
#include "dbclient.h"
#include "rest_biocard.h"

http_listener register_biocard(uri url)
{
    http_listener listener(url);
    listener.support(methods::GET, biocard_get);
    listener.support(methods::POST, biocard_post);
    listener.support(methods::PUT, biocard_put);
    listener.support(methods::PATCH, biocard_patch);

    listener
        .open()
        .then([&listener]() { cout << "starting to listen biocard" << endl; })
        .wait();

    return listener;
}

void biocard_get(http_request request)
{
    TRACE(L"GET biocard\n");

    http::status_code sc = status_codes::OK;

    handle_request(
        request,
        [&](json::value const& req_json, json::value& answer)
        {
            try
            {
                auto sp = uri::split_path(request.relative_uri().to_string());
                if (sp.size() != 1) throw runtime_error("GET biocard/{uuid} expected.");

                //string_generator gen;
                //uuid u = gen(ws2s(sp[0]));

                pqxx::connection c(db_connection_url);
                pqxx::work txn(c);

                /*pqxx::row r = txn.exec1(
                    "SELECT * " \
                    "FROM " + txn.quote(DB_BIOCARD_TABLE_NAME) +
                    "WHERE guid =" + txn.quote(sp[0]));
                */
            }
            catch (const boost::system::error_code& ec)
            {
                sc = status_codes::BadRequest;
                answer[ELEMENT_RESULT] = json::value::boolean(false);
                //std::string val = STD_TO_UTF(ec.message());
                //answer[ELEMENT_ERROR] = json::value::string(conversions::to_string_t(val));

                cout << "Exception: " << ec.message() << endl;
            }
            catch (const std::exception& ec)
            {
                sc = status_codes::BadRequest;
                answer[ELEMENT_RESULT] = json::value::boolean(false);
                //std::string val = STD_TO_UTF(ec.what());
                //answer[ELEMENT_ERROR] = json::value::string(conversions::to_string_t(val));

                cout << "Exception: " << ec.what() << endl;
            }
        });


    request.reply(sc, "");
}

void biocard_post(http_request request)
{
    TRACE(L"POST biocard\n");

    request.reply(status_codes::OK, "");
}

void biocard_put(http_request request)
{
    TRACE(L"PUT biocard\n");

    request.reply(status_codes::OK, "");
}

void biocard_patch(http_request request)
{
    TRACE(L"PATCH biocard\n");

    request.reply(status_codes::OK, "");
}
