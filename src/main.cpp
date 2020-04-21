#include "AbisRest.h"
#include "rest_biocard.h"
#include "rest_compare.h"
#include "rest_echo.h"
#include "rest_extract.h"
#include "rest_verify.h"
#include "rest_search.h"
#include "dbclient.h"
#include "restutils.h"

std::string db_connection_url("postgresql://");

void rest_server()
{
	uri_builder endpointBuilder;
	endpointBuilder.set_scheme(U("http"));
	endpointBuilder.set_port(10101);
#ifdef _WIN32
    endpointBuilder.set_host(U("*"));
#else
    endpointBuilder.set_host(U("0.0.0.0"));
#endif // _WIN32

	try
	{
        endpointBuilder.set_path(U("echo"));
        http_listener echo_listener = register_echo(endpointBuilder.to_uri());

        endpointBuilder.set_path(U("extract"));
        http_listener extract_listener = register_extract(endpointBuilder.to_uri());

        endpointBuilder.set_path(U("compare"));
        http_listener compare_listener = register_compare(endpointBuilder.to_uri());

        endpointBuilder.set_path(U("biocard"));
        http_listener biocard_listener = register_biocard(endpointBuilder.to_uri());

        endpointBuilder.set_path(U("verify"));
        http_listener verify_listener = register_verify(endpointBuilder.to_uri());

        endpointBuilder.set_path(U("search"));
        http_listener search_listener = register_search(endpointBuilder.to_uri());

		while (true);

		echo_listener.close().wait();
		extract_listener.close().wait();
		compare_listener.close().wait();
		biocard_listener.close().wait();
		verify_listener.close().wait();
		search_listener.close().wait();
	}
	catch (exception const& e)
	{
		cout << e.what() << endl;
	}
}

void dbinit()
{
    db_connection_url.append("postgresql:postgresql");
    db_connection_url.append("@localhost/");
    db_connection_url.append(DB_DATABASE_NAME);

    cout << "DB url: " << db_connection_url << endl;
}

int main()
{

	rest_server();

	return 0;
}