#include "rest_verify.h"
#include "AbisRest.h"
#include "restutils.h"
#include "dbclient.h"
#include "ebsclient.h"

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
BOOST_LOG_TRIVIAL(debug) << "verify_get: start";

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
						int json_tmp_type = ABIS_DATA;
						void* json_tmp_ptr = nullptr;

						if (tmp_from_json(arr[i], json_tmp_type, json_tmp_ptr) <= 0)
						{
							BOOST_LOG_TRIVIAL(debug) << "verify_get: error extract template";
							free(json_tmp_ptr);
							continue;
						}

						for (int r = 0; r < PQntuples(sql_res); r++)
						{
							int db_tmp_id = ntohl(*(int*)(PQgetvalue(sql_res, r, id_num)));
							int db_tmp_type = ntohl(*(int*)(PQgetvalue(sql_res, r, type_num)));

							if (db_tmp_type == ABIS_FACE_TEMPLATE && db_tmp_type == json_tmp_type)
							{
								void* arr_ptr = malloc(ABIS_TEMPLATE_SIZE);
								memset(arr_ptr, 0, ABIS_TEMPLATE_SIZE);

								float cmp = 0;
								int t = db_face_tmp_by_id(db, db_tmp_id, arr_ptr);
								if (t > 0) cmp = cmp_face_tmp(json_tmp_ptr, arr_ptr);
								free(arr_ptr);

								sw_trhld.push_back(ABIS_FACE_THRESHOLD);
								sw_score.push_back(cmp);
							}
							if (db_tmp_type == ABIS_FINGER_TEMPLATE && db_tmp_type == json_tmp_type)
							{
								void* arr_ptr = malloc(ABIS_TEMPLATE_SIZE);
								memset(arr_ptr, 0, ABIS_TEMPLATE_SIZE);

								float cmp = 0;
								int t = db_finger_tmp_by_id(db, db_tmp_id, arr_ptr);
								if (t > 0) cmp = cmp_finger_tmp(json_tmp_ptr, arr_ptr);
								free(arr_ptr);

								sw_trhld.push_back(ABIS_FINGER_THRESHOLD);
								sw_score.push_back(cmp);
							}
						}
BOOST_LOG_TRIVIAL(debug) << "verify_get: 1";
						if(json_tmp_ptr != nullptr) free(json_tmp_ptr);
					}
					float trhld = sw_trhld[0];
					float score = sw_score[0];
					for (size_t i = 1; i < sw_trhld.size(); i++)
					{
BOOST_LOG_TRIVIAL(debug) << "verify_get: 2";
						trhld = sugeno_weber(trhld, sw_trhld[i]);
BOOST_LOG_TRIVIAL(debug) << "verify_get: 3";
						score = sugeno_weber(score, sw_score[i]);
					}
					answer[ELEMENT_VALUE] = json::value::number(score * ABIS_INTEGRA_THRESHOLD / trhld);
					answer[ELEMENT_RESULT] = json::value::boolean(true);
BOOST_LOG_TRIVIAL(debug) << "verify_get: 4";
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
BOOST_LOG_TRIVIAL(debug) << "verify_get: end";
		request.reply(sc, "");
}
