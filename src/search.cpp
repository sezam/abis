#include "search.h"

http_listener register_search(uri url)
{
    http_listener listener(url);
    listener.support(methods::GET, search_get);

    listener
        .open()
        .then([&listener]() { cout << "starting to listen search" << endl; })
        .wait();

    return listener;
}

void search_get(http_request request)
{
    TRACE(L"GET search\n");

    request.reply(status_codes::OK, "");
}
