#include "verify.h"

http_listener register_verify(uri url)
{
	http_listener listener(url);
	listener.support(methods::GET, verify_get);
	return listener;
}

void verify_get(http_request request)
{
	TRACE(L"\nhandle verify GET\n");

	request.reply(status_codes::OK, "");
}
