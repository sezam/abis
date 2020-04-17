#include "AbisRest.h"
#include "extract.h"
#include "restutils.h"
#include "ebsclient.h"
#include "fplibclient.h"

http_listener register_extract(web::uri url)
{
	http_listener listener(url);
	listener.support(methods::GET, extract_get);
	return listener;
}

void extract_get(http_request request)
{
	TRACE(L"\nhandle extract GET\n");

	handle_request(
		request,
		[](json::value const& req_json, json::value& answer)
		{
			try
			{
				int element_type = ABIS_TYPE_DATA;

				json::value el_type = req_json.at(U("element_type"));
				if (el_type.is_integer()) element_type = el_type.as_integer();
				if (el_type.is_string()) element_type = stoi(el_type.as_string());

				auto element_image = req_json.at(U("element_image")).as_string();

				vector<unsigned char> buf = conversions::from_base64(element_image);
				if (element_type == ABIS_TYPE_FACE)
				{
					float* face_tmp = new float[FACE_TEMPLATE_SIZE];
					int count = 0;
					try
					{
						count = get_face_template(buf.data(), buf.size(), face_tmp, FACE_TEMPLATE_SIZE * sizeof(float));
					}
					catch (const std::exception&)
					{
						delete[] face_tmp;
						throw runtime_error("get_face_template");
					}

					if (count == 1)
					{
						for (size_t i = 0; i < FACE_TEMPLATE_SIZE; i++)
						{
							answer[U("element_vector")][i] = json::value::number(face_tmp[i]);
						}
					}
					else {
						answer[U("element_vector")] = json::value::object();
					}
					delete[] face_tmp;
				}

				if (element_type == ABIS_TYPE_FINGER)
				{
					unsigned char* finger_tmp = (unsigned char*)malloc(FINGER_TEMPLATE_SIZE);
					try
					{
						get_fingerprint_template(buf.data(), buf.size(), finger_tmp, FINGER_TEMPLATE_SIZE);

					}
					catch (const std::exception&)
					{
						free(finger_tmp);
						throw runtime_error("get_fingerprint_template");
					}

					for (size_t i = 0; i < FINGER_TEMPLATE_SIZE; i++)
					{
						answer[U("element_vector")][i] = json::value::number(finger_tmp[i]);
					}
					free(finger_tmp);
				}

				answer[U("ok")] = json::value::boolean(true);
				answer[U("element_type")] = json::value::string(conversions::to_string_t(to_string(element_type)));
			}
			catch (const boost::system::error_code& ec)
			{
				answer[U("ok")] = json::value::boolean(false);
				std::string val = STD_TO_UTF(ec.message());
				answer[U("error")] = json::value::string(conversions::to_string_t(val));
			}
			catch (const std::exception& ec)
			{
				answer[U("ok")] = json::value::boolean(false);
				std::string val = STD_TO_UTF(ec.what());
				answer[U("error")] = json::value::string(conversions::to_string_t(val));
			}
		});

	request.reply(status_codes::OK, "");
}
