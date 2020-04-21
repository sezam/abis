#include "rest_verify.h"

http_listener register_verify(uri url)
{
    http_listener listener(url);
    listener.support(methods::GET, verify_get);

    listener
        .open()
        .then([&listener]() { cout << "starting to listen verify" << endl; })
        .wait();

    return listener;
}

void verify_get(http_request request)
{
    TRACE(L"GET verify\n");

    request.reply(status_codes::OK, "");
}
