#include "AbisRest.h"
#include "rest_extract.h"
#include "restutils.h"
#include "fplibclient.h"

http_listener register_extract(web::uri url)
{
	http_listener listener(url);
	listener.support(methods::GET, extract_get);

	listener
		.open()
		.then([&listener]() { BOOST_LOG_TRIVIAL(info) << "starting to listen extact"; })
		.wait();

	return listener;
}

void extract_get(http_request request)
{
	http::status_code sc = status_codes::BadRequest;

	handle_request(
		request,
		[&](json::value const& req_json, json::value& answer)
		{
			try
			{
				int tmp_type = ABIS_DATA;
				void* tmp_ptr = nullptr;

				int res = tmp_from_json(req_json, tmp_type, tmp_ptr);
				bool step = res > 0;
				if (!step) BOOST_LOG_TRIVIAL(debug) << "extract_get: error extract template";


				if (step)
				{
					if (tmp_type == ABIS_FACE_TEMPLATE || tmp_type == ABIS_FINGER_TEMPLATE || tmp_type == ABIS_IRIS_TEMPLATE)
					{
						for (size_t i = 0; i < ABIS_TEMPLATE_LEN; i++)
							answer[ELEMENT_VALUE][i] = json::value::number(((float*)tmp_ptr)[i]);
					}
					if (tmp_type == ABIS_FINGER_GOST_TEMPLATE)
					{
						for (size_t i = 0; i < ABIS_FINGER_TEMPLATE_SIZE; i++)
							answer[ELEMENT_VALUE][i] = json::value::number(((char*)tmp_ptr)[i]);
					}
					if (tmp_type == ABIS_LIVEFACE_IMAGE)
					{
						answer[ELEMENT_VALUE] = json::value::number(*((int*)tmp_ptr));
					}
				}
				if (tmp_ptr != nullptr)free(tmp_ptr);

				if (!step) answer[ELEMENT_ERROR] = json::value::number(abs(res));
				answer[ELEMENT_TYPE] = json::value::number(tmp_type);
				answer[ELEMENT_RESULT] = json::value::boolean(step);

				sc = status_codes::OK;
			}
			catch (const boost::system::error_code& ec)
			{
				JSON_EXCEPTION(answer, ec.message());
			}
			catch (const std::exception& ec)
			{
				JSON_EXCEPTION(answer, ec.what());
			}
		});
	request.reply(sc, "");
}
