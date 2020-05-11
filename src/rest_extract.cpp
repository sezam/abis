#include "AbisRest.h"
#include "rest_extract.h"
#include "restutils.h"
#include "ebsclient.h"

http_listener register_extract(web::uri url)
{
	http_listener listener(url);
	listener.support(methods::GET, extract_get);

	listener
		.open()
		.then([&listener]() { cout << "starting to listen extact" << endl; })
		.wait();

	return listener;
}

void extract_get(http_request request)
{
	TRACE(L"GET extract\n");

	http::status_code sc = status_codes::OK;

	handle_request(
		request,
		[&](json::value const& req_json, json::value& answer)
		{
			try
			{
				auto element_image = req_json.at(ELEMENT_VALUE).as_string();
				vector<unsigned char> buf = conversions::from_base64(element_image);

				int element_type = req_json.at(ELEMENT_TYPE).as_integer();
				if (element_type == ABIS_FACE_IMAGE || element_type == ABIS_FINGER_IMAGE)
				{
					int template_type = ABIS_DATA;

					float* vec_tmp = (float*)malloc(ABIS_TEMPLATE_SIZE);
					memset(vec_tmp, 0, ABIS_TEMPLATE_SIZE);

					int res = 0;
					if (element_type == ABIS_FACE_IMAGE)
					{
						template_type = ABIS_FACE_TEMPLATE;
						res = extract_face_template(buf.data(), buf.size(), vec_tmp, ABIS_TEMPLATE_SIZE);
					}
					if (element_type == ABIS_FINGER_IMAGE)
					{
						template_type = ABIS_FINGER_TEMPLATE;
						res = extract_finger_template(buf.data(), buf.size(), vec_tmp, ABIS_TEMPLATE_SIZE);
					}
					if (res == 1)
					{
						for (size_t i = 0; i < ABIS_TEMPLATE_LEN; i++)
							answer[ELEMENT_VALUE][i] = json::value::number(vec_tmp[i]);
					}
					free(vec_tmp);

					answer[ELEMENT_TYPE] = json::value::number(template_type);
					answer[ELEMENT_RESULT] = json::value::boolean(res > 0);
				}
			}
			catch (const boost::system::error_code& ec)
			{
				sc = status_codes::BadRequest;
				answer[ELEMENT_RESULT] = json::value::boolean(false);
				cout << "Exception: " << ec.message() << endl;
			}
			catch (const std::exception& ec)
			{
				sc = status_codes::BadRequest;
				answer[ELEMENT_RESULT] = json::value::boolean(false);
				cout << "Exception: " << ec.what() << endl;
			}
		});

	request.reply(sc, "");
}
