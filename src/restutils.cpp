#include "restutils.h"

std::wstring s2ws(const std::string& str)
{
	return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
}

std::string ws2s(const std::wstring& wstr)
{
	return utf_to_utf<char>(wstr.c_str(), wstr.c_str() + wstr.size());
}

void display_json(json::value const& jvalue, std::string const& prefix)
{
	//cout << prefix << jvalue.serialize() << endl;

	cout << prefix << endl;

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