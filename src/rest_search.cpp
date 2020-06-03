#include "AbisRest.h"
#include "rest_search.h"
#include "restutils.h"
#include "dbclient.h"
#include "ebsclient.h"
#include "fplibclient.h"

http_listener register_search(uri url)
{
	http_listener listener(url);
	listener.support(methods::GET, search_get);

	listener
		.open()
		.then([&listener]() { BOOST_LOG_TRIVIAL(info) << "starting to listen search"; })
		.wait();

	return listener;
}

void search_get(http_request request)
{
	http::status_code sc = status_codes::BadRequest;

	handle_request(
		request,
		[&](json::value const& req_json, json::value& answer)
		{
			PGconn* db = nullptr;
			try
			{
				db = db_open();

				json::array arr = req_json.as_array();
				auto json_out = json::value::array();
				for (size_t i = 0; i < arr.size(); i++)
				{
					auto json_row = json::value::object();
					int tmp_type = ABIS_DATA;
					void* tmp_in = nullptr;
					void* tmp_gost = nullptr;
					int tmp_id = 0;
					float score = 0;
					bool step = false;

					int element_type = arr[i].at(ELEMENT_TYPE).as_integer();
					if (element_type == ABIS_FACE_IMAGE || element_type == ABIS_FACE_TEMPLATE)
					{
						step = face_tmp_from_json(arr[i], tmp_type, tmp_in) > 0;
						if (!step) BOOST_LOG_TRIVIAL(debug) << "search_get: error extract face template";
					}

					if (element_type == ABIS_FINGER_IMAGE || element_type == ABIS_FINGER_GOST_IMAGE)
					{
						tmp_type = ABIS_FINGER_TEMPLATE;
						step = finger_tmp_from_json(arr[i], tmp_in, tmp_gost) > 0;
						if (!step) BOOST_LOG_TRIVIAL(debug) << "search_get: error extract finger template";
					}

					if (tmp_type == ABIS_FACE_TEMPLATE)
					{
						if (step)
						{
							tmp_id = db_search_face_tmp(db, tmp_in);
							step = tmp_id > 0;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "search_get: error search face template";
						}
						if (step)
						{
							void* tmp_db = malloc(ABIS_TEMPLATE_SIZE);
							step = tmp_db != nullptr;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "search_get: error memory allocate 1";

							if (step)
							{
								memset(tmp_db, 0, ABIS_TEMPLATE_SIZE);
								step = db_face_tmp_by_id(db, tmp_id, tmp_db) > 0;
								if (!step) BOOST_LOG_TRIVIAL(debug) << "search_get: error get face template";
							}

							if (step) score = cmp_face_tmp(tmp_in, tmp_db);
							if (tmp_db != nullptr) free(tmp_db);
						}
					}
					if (tmp_type == ABIS_FINGER_TEMPLATE)
					{
						if (step)
						{
							tmp_id = db_search_finger_tmp(db, tmp_in);
							step = tmp_id > 0;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "search_get: error search finger template";
						}
						if (step)
						{
							void* gost_db = malloc(ABIS_FINGER_TMP_GOST_SIZE);
							step = gost_db != nullptr;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "search_get: error memory allocate 2";

							if (step)
							{
								memset(gost_db, 0, ABIS_FINGER_TMP_GOST_SIZE);

								step = db_gost_tmp_by_id(db, tmp_id, gost_db) > 0;
								if (!step) BOOST_LOG_TRIVIAL(debug) << "search_get: error get finger template phase 2";
							}
							if (step) score = cmp_fingerprint_gost_template(tmp_gost, gost_db);
							if (gost_db != nullptr) free(gost_db);
						}
					}

					if (step)
					{
						json_row[ELEMENT_ID] = json::value::number(tmp_id);

						char bc_gid[50];
						step = db_card_id_by_tmp_id(db, tmp_type, tmp_id, bc_gid) > 0;
						if (step)
						{
							json_row[ELEMENT_UUID] = json::value::string(conversions::to_string_t(bc_gid));
							json_row[ELEMENT_VALUE] = json::value::number(score);
						}
					}
					json_row[ELEMENT_RESULT] = json::value::boolean(step);
					json_row[ELEMENT_TYPE] = json::value::number(tmp_type);

					json_out[i] = json_row;
					if (tmp_in != nullptr) free(tmp_in);
					if (tmp_gost != nullptr) free(tmp_gost);
				}

				answer[ELEMENT_VALUE] = json_out;
				answer[ELEMENT_RESULT] = json::value::boolean(true);
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

			db_close(db);
		});
	request.reply(sc, "");
}
