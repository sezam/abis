#include "restutils.h"

std::wstring s2ws(const std::string& str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

void display_json(json::value const& jvalue, std::string const& prefix)
{
	//cout << prefix << jvalue.serialize() << endl;

	cout << prefix << endl;

	if (jvalue.is_object())
	{
		for (auto const& e : jvalue.as_object())
		{
			cout << "\t{ " << ws2s(e.first.substr(0, 20));
			cout << " : \t" << ws2s(e.second.to_string().substr(0, 50)) << "}" << endl;
		}
	}
	if (jvalue.is_array())
	{
		cout << "  [";
		for (auto const& e : jvalue.as_array())
		{
			display_json(e, "");
		}
		cout << "  ]" << endl;
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
						display_json(jvalue, "Recv: ");

						if (!jvalue.is_null())
						{
							action(jvalue, answer);
						}
					}
					catch (http_exception const& e)
					{
						cout << e.what() << endl;
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

	display_json(answer, "Send: ");
	request.reply(status_codes::OK, answer);
	}