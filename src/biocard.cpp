#include "biocard.h"
#include "AbisRest.h"

http_listener register_biocard(uri url)
{
	http_listener listener(url);
	listener.support(methods::GET, biocard_get);
	listener.support(methods::POST, biocard_post);
	listener.support(methods::PUT, biocard_put);
	listener.support(methods::PATCH, biocard_patch);
	return listener;
}

void biocard_get(http_request request)
{
	TRACE(L"\nhandle biocard GET\n");

	request.reply(status_codes::OK, "");
}

void biocard_post(http_request request)
{
	TRACE(L"\nhandle biocard POST\n");

	request.reply(status_codes::OK, "");
}

void biocard_put(http_request request)
{
	TRACE(L"\nhandle biocard PUT\n");

	request.reply(status_codes::OK, "");
}

void biocard_patch(http_request request)
{
	TRACE(L"\nhandle biocard PATCH\n");

	request.reply(status_codes::OK, "");
}
