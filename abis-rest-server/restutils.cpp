#include "restutils.h"

void display_json(
	json::value const & jvalue,
	utility::string_t const & prefix)
{
	//wcout << prefix << jvalue.serialize() << endl;

	wcout << prefix << endl;
	for (auto const & e : jvalue.as_object())
	{
		wcout << "\t" << e.first << endl;
	}
}

void handle_request(
	http_request request,
	function<void(json::value const &, json::value &)> action)
{
	auto answer = json::value::object();

	request
		.extract_json()
		.then([&answer, &action](pplx::task<json::value> task) {
		try
		{
			auto const & jvalue = task.get();
			display_json(jvalue, L"Recv: ");

			if (!jvalue.is_null())
			{
				action(jvalue, answer);
			}
		}
		catch (http_exception const & e)
		{
			wcout << e.what() << endl;
		}
	})
		.wait();


	display_json(answer, L"Send: ");

	request.reply(status_codes::OK, answer);
}