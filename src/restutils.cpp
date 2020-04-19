#include "restutils.h"

void display_json(
	json::value const& jvalue,
	utility::string_t const& prefix)
{
	//wcout << prefix << jvalue.serialize() << endl;

	wcout << prefix << endl;

	if (jvalue.is_object())
	{
		for (auto const& e : jvalue.as_object())
		{
			wcout << "\t{ " << e.first.substr(0, 20);
			wcout << " : \t" << e.second.to_string().substr(0, 50) << "}" << endl;
		}
	}
	if (jvalue.is_array())
	{
		wcout << "  [";
		for (auto const& e : jvalue.as_array())
		{
			display_json(e, U(""));
		}
		wcout << "  ]" << endl;
	}
}

void handle_request(
	http_request request,
	function<void(json::value const&, json::value&)> action)
{
	auto answer = json::value::object();

	try
	{
		request
			.extract_json()
			.then([&answer, &action](pplx::task<json::value> task)
				{
					try
					{
						auto const& jvalue = task.get();
						display_json(jvalue, L"Recv: ");

						if (!jvalue.is_null())
						{
							action(jvalue, answer);
						}
					}
					catch (http_exception const& e)
					{
						wcout << e.what() << endl;
					}
				})
			.wait();
	}
	catch (const std::exception& ec)
	{
#ifdef _DEBUG
		answer[U("exception")] = json::value::string(conversions::to_string_t(ec.what()));
#endif // DEBUG
	}

	display_json(answer, L"Send: ");
	request.reply(status_codes::OK, answer);
}