#include "restutils.h"
#include "echotest.h"

http_listener register_echo(uri url)
{
    http_listener listener(url);

    listener.support(methods::GET, echo_get);
    listener.support(methods::POST, echo_post);
    listener.support(methods::PUT, echo_put);
    listener.support(methods::DEL, echo_del);

    listener
        .open()
        .then([&listener]() { cout << "starting to listen echo" << endl; })
        .wait();

    return listener;
}

void echo_get(http_request request)
{
    TRACE("GET echo\n");

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
    TRACE("POST echo\n");

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
    TRACE("PUT echo\n");

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
    TRACE("DEL echo\n");

    handle_request(
        request,
        [](json::value const& jvalue, json::value& answer) {
            answer = jvalue;
            display_json(jvalue, "R: ");
            display_json(answer, "S: ");
        });
}