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
					vector<float> face_scores;
					vector<float> finger_scores;

					for (size_t i = 0; i < arr.size(); i++)
					{
						void* json_tmp_ptr = nullptr;
						int json_tmp_type = ABIS_DATA;
						int json_tmp_id = 0;

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

									if (step)
									{
										memset(db_tmp_ptr, 0, ABIS_TEMPLATE_SIZE);

										step = db_face_tmp_by_id(db, db_tmp_id, db_tmp_ptr) > 0;
										if (!step) BOOST_LOG_TRIVIAL(debug) << "verify_get: error get face template";
									}

									if (step) face_scores.push_back(cmp_face_tmp(json_tmp_ptr, db_tmp_ptr));
									if (db_tmp_ptr != nullptr) free(db_tmp_ptr);
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
									if (step) finger_scores.push_back(cmp_fingerprint_gost_template(((uchar*)json_tmp_ptr) + ABIS_TEMPLATE_SIZE, gost_db));
									if (gost_db != nullptr) free(gost_db);
								}
							}
						}
						if (json_tmp_ptr != nullptr) free(json_tmp_ptr);
					}

					float face_score = 0.f;
					float face_thrh = ABIS_FACE_THRESHOLD;
					if (!face_scores.empty())
					{
						face_score = face_scores[0];
						for (auto fs : face_scores)
						{
							face_thrh = sugeno_weber(face_thrh, ABIS_FACE_THRESHOLD);
							face_score = sugeno_weber(face_score, fs);
						}
					}

					float finger_score = 0.f;
					float finger_thrh = ABIS_FINGER_GOST_THRESHOLD;
					if (!finger_scores.empty())
					{
						finger_score = finger_scores[0];
						for (auto fs : finger_scores)
						{
							finger_thrh = sugeno_weber(finger_thrh, ABIS_FINGER_GOST_THRESHOLD);
							finger_score = sugeno_weber(finger_score, fs);
						}
					}

					float score = 0.f;
					if (face_score > 0.f && finger_score > 0.f)
					{
						float trhld = sugeno_weber(face_thrh, finger_thrh);
						score = sugeno_weber(face_score, finger_score);
						score = min(score * ABIS_INTEGRA_THRESHOLD / trhld, 1.0f);
					}
					else
					{
						if (face_score > 0.f) score = min(face_score * ABIS_INTEGRA_THRESHOLD / face_thrh, 1.0f);
						if (finger_score > 0.f) score = min(finger_score * ABIS_INTEGRA_THRESHOLD / finger_thrh, 1.0f);
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
