#include "AbisRest.h"
#include "extract.h"
#include "restutils.h"
#include "ebsclient.h"

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
				int element_type = ABIS_BIO_DATA;

				json::value el_type = req_json.at(U("element_type"));
				if (el_type.is_integer()) element_type = el_type.as_integer();
				if (el_type.is_string()) element_type = stoi(el_type.as_string());

				auto element_image = req_json.at(U("element_image")).as_string();

				if (element_type == ABIS_BIO_FACE) {
					vector<unsigned char> buf = conversions::from_base64(element_image);

					float* face_tmp = new float[FACESIZE];
					int count = get_face_template(&buf[0], buf.size(), face_tmp, sizeof(float) * FACESIZE);

					if (count != 1) {
						delete[] face_tmp;
						throw(boost::system::errc::make_error_code(boost::system::errc::no_message));
					}
					for (size_t i = 0; i < FACESIZE; i++)
					{
						answer[U("element_vector")][i] = json::value::number(face_tmp[i]);
					}
					delete[] face_tmp;
					answer[U("element_type")] = json::value::string(conversions::to_string_t(to_string(element_type)));
				}
				answer[U("ok")] = json::value::boolean(true);
			}
			catch (boost::system::error_code ec)
			{
				answer[U("ok")] = json::value::boolean(false);
				std::string val = STD_TO_UTF(ec.message());
				answer[U("error")] = json::value::string(conversions::to_string_t(val));
			}
		});

	request.reply(status_codes::OK, "");
}
