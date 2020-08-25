#include "AbisRest.h"
#include "rest_biocard.h"
#include "rest_compare.h"
#include "rest_echo.h"
#include "rest_extract.h"
#include "rest_verify.h"
#include "rest_search.h"
#include "dbclient.h"
#include "restutils.h"
#include "liveclient.h"

#include <pplx/threadpool.h>

int debug;

void goznak_rest_server()
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
		BOOST_LOG_TRIVIAL(error) << e.what();
	}
}

static const std::string COMMON_FMT("[%TimeStamp%][%ThreadID%][%Severity%]:  %Message%");

void logging_prepare()
{
	logging::trivial::severity_level lvl;
	logging::trivial::from_string(logging_level.c_str(), logging_level.length(), lvl);
	logging::core::get()->set_filter
	(
		logging::trivial::severity >= lvl
	);
	logging::add_console_log(
		std::cout,
		keywords::auto_flush = true,
		keywords::format = COMMON_FMT
	);
	logging::add_file_log
	(
		keywords::file_name = logging_path + "/abis-rest-server_%N.log", 
		keywords::rotation_size = 10 * 1024 * 1024, 
		keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), 
		keywords::auto_flush = true,
		keywords::format = COMMON_FMT
	);
	logging::add_common_attributes();
}

int main(int argc, char* argv[])
{
	load_settings(argv[1]);

	crossplat::threadpool::initialize_with_threads(5);

	logging_prepare();
	db_prepare();
	live_prepare();

	goznak_rest_server();

	live_free();
	return 0;
}