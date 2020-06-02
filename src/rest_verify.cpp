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
					vector<float> sw_trhld;
					vector<float> sw_score;
					for (size_t i = 0; i < arr.size(); i++)
					{
						void* tmp_in = nullptr;
						void* tmp_gost = nullptr;
						int tmp_type = ABIS_DATA;
						int tmp_id = 0;
						float score = 0;
						bool step = false;

						int element_type = arr[i].at(ELEMENT_TYPE).as_integer();
						if (element_type == ABIS_FACE_IMAGE || element_type == ABIS_FACE_TEMPLATE)
						{
							step = face_tmp_from_json(arr[i], tmp_type, tmp_in) > 0;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "verify_get: error extract face template";
						}

						if (element_type == ABIS_FINGER_IMAGE || element_type == ABIS_FINGER_GOST_IMAGE)
						{
							tmp_type = ABIS_FINGER_TEMPLATE;
							step = finger_tmp_from_json(arr[i], tmp_in, tmp_gost) > 0;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "verify_get: error extract finger template";
						}

						for (int r = 0; r < PQntuples(sql_res); r++)
						{
							int db_tmp_id = ntohl(*(int*)(PQgetvalue(sql_res, r, id_num)));
							int db_tmp_type = ntohl(*(int*)(PQgetvalue(sql_res, r, type_num)));

							if (db_tmp_type == ABIS_FACE_TEMPLATE && db_tmp_type == tmp_type)
							{
								void* tmp_db = malloc(ABIS_TEMPLATE_SIZE);
								step = tmp_db != nullptr;
								if (!step) BOOST_LOG_TRIVIAL(debug) << "verify_get: error memory allocate 1";

								float score = 0;
								if (step)
								{
									memset(tmp_db, 0, ABIS_TEMPLATE_SIZE);

									step = db_face_tmp_by_id(db, db_tmp_id, tmp_db) > 0;
									if (!step) BOOST_LOG_TRIVIAL(debug) << "verify_get: error get face template";
								}

								if (step) score = cmp_face_tmp(tmp_in, tmp_db);
								if (tmp_db != nullptr) free(tmp_db);

								sw_trhld.push_back(ABIS_FACE_THRESHOLD);
								sw_score.push_back(score);
							}
							if (db_tmp_type == ABIS_FINGER_TEMPLATE && db_tmp_type == tmp_type)
							{
								void* gost_db = malloc(ABIS_FINGER_TMP_GOST_SIZE);
								step = gost_db != nullptr;
								if (!step) BOOST_LOG_TRIVIAL(debug) << "verify_get: error memory allocate 2";

								if (step)
								{
									memset(gost_db, 0, ABIS_FINGER_TMP_GOST_SIZE);

									step = db_gost_tmp_by_id(db, tmp_id, gost_db) > 0;
									if (!step) BOOST_LOG_TRIVIAL(debug) << "verify_get: error get finger template";
								}
								if (step) score = cmp_fingerprint_gost_template(tmp_gost, gost_db);
								if (gost_db != nullptr) free(gost_db);

								sw_trhld.push_back(ABIS_FINGER_THRESHOLD);
								sw_score.push_back(score);
							}
						}
						if (tmp_in != nullptr) free(tmp_in);
						if (tmp_gost != nullptr) free(tmp_gost);
					}

					float trhld = sw_trhld[0];
					float score = sw_score[0];
					for (size_t i = 1; i < sw_trhld.size(); i++)
					{
						trhld = sugeno_weber(trhld, sw_trhld[i]);
						score = sugeno_weber(score, sw_score[i]);
					}
					answer[ELEMENT_VALUE] = json::value::number(score * ABIS_INTEGRA_THRESHOLD / trhld);
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
