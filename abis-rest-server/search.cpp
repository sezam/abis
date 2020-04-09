#include "search.h"

http_listener register_search(uri url)
{
	http_listener listener(url);
	listener.support(methods::GET, search_get);
	return listener;
}

void search_get(http_request request)
{
	TRACE(L"\nhandle search GET\n");

	request.reply(status_codes::OK, "");
}
