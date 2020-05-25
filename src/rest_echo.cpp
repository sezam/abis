#include "restutils.h"
#include "rest_echo.h"

http_listener register_echo(uri url)
{
    http_listener listener(url);

    listener.support(methods::GET, echo_get);
    listener.support(methods::POST, echo_post);
    listener.support(methods::PUT, echo_put);
    listener.support(methods::DEL, echo_del);

    listener
        .open()
        .then([&listener]() { BOOST_LOG_TRIVIAL(info) << "starting to listen echo"; })
        .wait();

    return listener;
}

void echo_get(http_request request)
{
    handle_request(
        request,
        [](json::value const& jvalue, json::value& answer) {
            answer = jvalue;
            display_json(jvalue, "R: ");
            display_json(answer, "S: ");
        });
}

void echo_post(http_request request)
{
    handle_request(
        request,
        [](json::value const& jvalue, json::value& answer) {
            answer = jvalue;
            display_json(jvalue, "R: ");
            display_json(answer, "S: ");
        });
}

void echo_put(http_request request)
{
    handle_request(
        request,
        [](json::value const& jvalue, json::value& answer) {
            answer = jvalue;
            display_json(jvalue, "R: ");
            display_json(answer, "S: ");
        });
}

void echo_del(http_request request)
{
    handle_request(
        request,
        [](json::value const& jvalue, json::value& answer) {
            answer = jvalue;
            display_json(jvalue, "R: ");
            display_json(answer, "S: ");
        });
}