#include "AbisRest.h"
#include "rest_verify.h"
#include "restutils.h"
#include "dbclient.h"
#include "ebsclient.h"
#include "fplibclient.h"

http_listener register_verify(uri url)
{
	http_listener listener(url);
	listener.support(methods::GET, verify_get);

	listener
		.open()
		.then([&listener]() { BOOST_LOG_TRIVIAL(info) << "starting to listen verify"; })
		.wait();

	return listener;
}

void verify_get(http_request request)
{
	http::status_code sc = status_codes::BadRequest;

	handle_request(
		request,
		[&](json::value const& req_json, json::value& answer)
		{
			PGconn* db = nullptr;
			PGresult* sql_res = nullptr;
			try
			{
				db = db_open();

				auto sp = uri::split_path(request.relative_uri().to_string());
				if (sp.size() != 1) throw runtime_error("GET verify/{uuid} expected.");

				string gids = utility::conversions::to_utf8string(sp[0]);
				string_generator gen;
				uuid gid = gen(gids);

				string s1 = to_string(gid);
				const char* paramValues[1] = { s1.c_str() };

				sql_res = db_exec_param(db, SQL_TMP_IDS_BY_BC_GID, 1, paramValues, 1);
				if (PQresultStatus(sql_res) == PGRES_TUPLES_OK && PQntuples(sql_res) > 0)
				{
					int id_num = PQfnumber(sql_res, "tmp_id");
					int type_num = PQfnumber(sql_res, "tmp_type");

					json::array arr = req_json.as_array();
					float score_finger = 0.f;
					float score_face = 0.f;

					for (size_t i = 0; i < arr.size(); i++)
					{
						void* json_tmp_ptr = nullptr;
						int json_tmp_type = ABIS_DATA;
						int json_tmp_id = 0;
						float score = 0;

						int res = tmp_from_json(arr[i], json_tmp_type, json_tmp_ptr);
						bool step = res > 0;

						if (!step) BOOST_LOG_TRIVIAL(debug) << "verify_get: error extract template";

						if (step)
						{
							for (int r = 0; r < PQntuples(sql_res); r++)
							{
								int db_tmp_id = ntohl(*(int*)(PQgetvalue(sql_res, r, id_num)));
								int db_tmp_type = ntohl(*(int*)(PQgetvalue(sql_res, r, type_num)));

								if (db_tmp_type == ABIS_FACE_TEMPLATE && db_tmp_type == json_tmp_type)
								{
									void* db_tmp_ptr = malloc(ABIS_TEMPLATE_SIZE);
									step = db_tmp_ptr != nullptr;
									if (!step) BOOST_LOG_TRIVIAL(debug) << "verify_get: error memory allocate 1";

									float score = 0;
									if (step)
									{
										memset(db_tmp_ptr, 0, ABIS_TEMPLATE_SIZE);

										step = db_face_tmp_by_id(db, db_tmp_id, db_tmp_ptr) > 0;
										if (!step) BOOST_LOG_TRIVIAL(debug) << "verify_get: error get face template";
									}

									if (step) score = cmp_face_tmp(json_tmp_ptr, db_tmp_ptr);
									if (db_tmp_ptr != nullptr) free(db_tmp_ptr);

									score_face = max(score_face, score);
								}
								if (db_tmp_type == ABIS_FINGER_GOST_TEMPLATE && db_tmp_type == json_tmp_type)
								{
									void* gost_db = malloc(ABIS_FINGER_TMP_GOST_SIZE);
									step = gost_db != nullptr;
									if (!step) BOOST_LOG_TRIVIAL(debug) << "verify_get: error memory allocate 2";

									if (step)
									{
										memset(gost_db, 0, ABIS_FINGER_TMP_GOST_SIZE);

										step = db_gost_tmp_by_id(db, db_tmp_id, gost_db) > 0;
										if (!step) BOOST_LOG_TRIVIAL(debug) << "verify_get: error get finger template";
									}
									if (step) score = cmp_fingerprint_gost_template(((uchar*)json_tmp_ptr) + ABIS_TEMPLATE_SIZE, gost_db);
									if (gost_db != nullptr) free(gost_db);

									score_finger = max(score_finger, score);
								}
							}
						}
						if (json_tmp_ptr != nullptr) free(json_tmp_ptr);
					}

					float score = 0.f;
					if (score_face > 0.f && score_finger > 0.f)
					{
						float trhld = sugeno_weber(ABIS_FACE_THRESHOLD, ABIS_FINGER_GOST_THRESHOLD);
						score = sugeno_weber(score_face, score_finger);
						score = min(score * ABIS_INTEGRA_THRESHOLD / trhld, 1.0f);
					}
					else
					{
						if (score_face > 0.f) score = min(score_face * ABIS_INTEGRA_THRESHOLD / ABIS_FACE_THRESHOLD, 1.0f);
						if (score_finger > 0.f) score = min(score_finger * ABIS_INTEGRA_THRESHOLD / ABIS_FINGER_GOST_THRESHOLD, 1.0f);
					}

					answer[ELEMENT_VALUE] = json::value::number(score);
					answer[ELEMENT_RESULT] = json::value::boolean(true);
				}
				else answer[ELEMENT_RESULT] = json::value::boolean(false);
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
			PQclear(sql_res);
			db_close(db);
		});
	request.reply(sc, "");
}
