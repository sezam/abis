#include "AbisRest.h"
#include "extract.h"
#include "compare.h"
#include "biocard.h"
#include "verify.h"
#include "search.h"
#include "ebsclient.h"
#include "restutils.h"

map<utility::string_t, utility::string_t> dictionary;

void handle_get(http_request request)
{
	TRACE(L"\nhandle GET\n");

	auto answer = json::value::object();

	for (auto const& p : dictionary)
	{
		answer[p.first] = json::value::string(p.second);
	}

	display_json(json::value::null(), U("R: "));
	display_json(answer, U("S: "));

	request.reply(status_codes::OK, answer);
}

void handle_post(http_request request)
{
	TRACE("\nhandle POST\n");

	handle_request(
		request,
		[](json::value const& jvalue, json::value& answer) {
			for (auto const& e : jvalue.as_array())
			{
				if (e.is_string())
				{
					auto key = e.as_string();
					auto pos = dictionary.find(key);

					if (pos == dictionary.end())
					{
						answer[key] = json::value::string(U("<nil>"));
					}
					else
					{
						answer[pos->first] = json::value::string(pos->second);
					}
				}
			}
		});
}

void handle_put(http_request request)
{
	TRACE("\nhandle PUT\n");

	handle_request(
		request,
		[](json::value const& jvalue, json::value& answer) {
			for (auto const& e : jvalue.as_object())
			{
				if (e.second.is_string())
				{
					auto key = e.first;
					auto value = e.second.as_string();

					if (dictionary.find(key) == dictionary.end())
					{
						TRACE_ACTION("added", key, value);
						answer[key] = json::value::string(U("<put>"));
					}
					else
					{
						TRACE_ACTION("updated", key, value);
						answer[key] = json::value::string(U("<updated>"));
					}

					dictionary[key] = value;
				}
			}
		});
}

void handle_del(http_request request)
{
	TRACE("\nhandle DEL\n");

	handle_request(
		request,
		[](json::value const& jvalue, json::value& answer) {
			set<utility::string_t> keys;
			for (auto const& e : jvalue.as_array())
			{
				if (e.is_string())
				{
					auto key = e.as_string();

					auto pos = dictionary.find(key);
					if (pos == dictionary.end())
					{
						answer[key] = json::value::string(U("<failed>"));
					}
					else
					{
						TRACE_ACTION("deleted", pos->first, pos->second);
						answer[key] = json::value::string(U("<deleted>"));
						keys.insert(key);
					}
				}
			}

			for (auto const& key : keys)
				dictionary.erase(key);
		});
}

void rest_server()
{
	http_listener listener(U("http://localhost/restdemo"));

	listener.support(methods::GET, handle_get);
	listener.support(methods::POST, handle_post);
	listener.support(methods::PUT, handle_put);
	listener.support(methods::DEL, handle_del);

	http_listener extract_listener = register_extract(U("http://localhost/extract"));
	http_listener compare_listener = register_compare(U("http://localhost/compare"));
	http_listener biocard_listener = register_biocard(U("http://localhost/biocard"));
	http_listener verify_listener = register_verify(U("http://localhost/verify"));
	http_listener search_listener = register_search(U("http://localhost/search"));

	try
	{
		listener
			.open()
			.then([&listener]() { TRACE("\nstarting to listen demo\n"); })
			.wait();

		extract_listener
			.open()
			.then([&extract_listener]() { TRACE("\nstarting to listen extract\n"); })
			.wait();

		compare_listener
			.open()
			.then([&compare_listener]() { TRACE("\nstarting to listen compare\n"); })
			.wait();

		biocard_listener
			.open()
			.then([&compare_listener]() { TRACE("\nstarting to listen biocard\n"); })
			.wait();

		verify_listener
			.open()
			.then([&compare_listener]() { TRACE("\nstarting to listen verify\n"); })
			.wait();

		search_listener
			.open()
			.then([&compare_listener]() { TRACE("\nstarting to listen search\n"); })
			.wait();

		while (true);

		listener.close().wait();
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

void test_ebs_client()
{
	FILE* file;
	unsigned char* buffer;
	unsigned long fileLen;

	file = fopen("c:\\temp\\1\\1.png", "rb");
	if (!file)
	{
		fprintf(stderr, "can't open file %s", "c:\\temp\\1\\1.png");
		exit(1);
	}

	fseek(file, 0, SEEK_END);
	fileLen = ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer = (unsigned char*)malloc(fileLen + 1);

	if (!buffer)
	{
		fprintf(stderr, "Memory error!");
		fclose(file);
		exit(1);
	}

	fread(buffer, fileLen, 1, file);
	fclose(file);

	float biotemplate[FACE_TEMPLATE_SIZE];
	int res = get_face_template(buffer, fileLen + 1, biotemplate, sizeof(biotemplate));
	printf("result is %d \n", res);

	free(buffer);
}

int main()
{
	//test_ebs_client();
	rest_server();

	return 0;
}