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

int debug1;

void test_extract_face()
{
	fstream fs("../test/face.base64");
	if (!fs.is_open()) {
		BOOST_LOG_TRIVIAL(debug) << "test_extract_face: file error";
		return;
	}

	//vector<unsigned char> buf = conversions::from_base64(fs);

	while (true)
	{

	}
}


void tst()
{
	try
	{
		test_extract_face();
	}
	catch (exception const& e)
	{
		BOOST_LOG_TRIVIAL(error) << e.what();
	}
}

static const std::string COMMON_FMT("[%TimeStamp%][%ThreadID%][%Severity%]:  %Message%");

void logging_prepare1()
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

int main1(int argc, char* argv[])
{
	load_settings(argv[1]);

	logging_prepare1();
	db_prepare();
	live_prepare();

	tst();

	live_free();
	return 0;
}