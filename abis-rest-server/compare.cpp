#include "compare.h"
#include "AbisRest.h"

http_listener register_compare(uri url)
{
	http_listener listener(url);
	listener.support(methods::GET, compare_get);
	return listener;
}

void compare_get(http_request request)
{
	TRACE(L"\nhandle compare GET\n");

	request.reply(status_codes::OK, "");
}
