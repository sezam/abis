#include "biocard.h"
#include "AbisRest.h"

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

	request.reply(status_codes::OK, "");
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
