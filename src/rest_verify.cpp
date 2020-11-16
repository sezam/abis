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
				logging_res(__func__, sql_res);

				if (PQresultStatus(sql_res) == PGRES_TUPLES_OK && PQntuples(sql_res) > 0)
				{
					int id_num = PQfnumber(sql_res, "tmp_id");
					int type_num = PQfnumber(sql_res, "tmp_type");

					json::array arr = req_json.as_array();
					vector<float> face_scores;
					vector<float> finger_scores;
					vector<float> iris_scores;

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
							float face_cmp_score = 0.f;
							float finger_cmp_score = 0.f;
							float iris_cmp_score = 0.f;
							for (int r = 0; r < PQntuples(sql_res); r++)
							{
								int db_tmp_id = ntohl(*(int*)(PQgetvalue(sql_res, r, id_num)));
								int db_tmp_type = ntohl(*(int*)(PQgetvalue(sql_res, r, type_num)));

								if (json_tmp_type == ABIS_FACE_TEMPLATE) 
								{
									float score = ABIS_FLOAT_THRESHOLD;
									if (db_tmp_type == json_tmp_type) step = db_tmp_cmp_by_id(db, db_tmp_type, json_tmp_ptr, db_tmp_id, score) > 0;
									face_cmp_score = max(face_cmp_score, score);
								}
								if (json_tmp_type == ABIS_FINGER_GOST_TEMPLATE)
								{
									float score = ABIS_FLOAT_THRESHOLD;
									if (db_tmp_type == json_tmp_type) step = db_tmp_cmp_by_id(db, db_tmp_type, json_tmp_ptr, db_tmp_id, score) > 0;
									finger_cmp_score = max(finger_cmp_score, score);
								}
								if (json_tmp_type == ABIS_IRIS_TEMPLATE)
								{
									float score = ABIS_FLOAT_THRESHOLD;
									if (db_tmp_type == json_tmp_type) step = db_tmp_cmp_by_id(db, db_tmp_type, json_tmp_ptr, db_tmp_id, score) > 0;
									iris_cmp_score = max(iris_cmp_score, score);
								}
							}
							if (face_cmp_score >= ABIS_FLOAT_THRESHOLD) face_scores.push_back(face_cmp_score);
							if (finger_cmp_score >= ABIS_FLOAT_THRESHOLD) finger_scores.push_back(finger_cmp_score);
							if (iris_cmp_score >= ABIS_FLOAT_THRESHOLD) iris_scores.push_back(iris_cmp_score);
						}
						if (json_tmp_ptr != nullptr) free(json_tmp_ptr);
					}

					vector<float> scores;
					if (!face_scores.empty()) scores.push_back(calc_score(face_scores, ABIS_FACE_THRESHOLD));
					if (!finger_scores.empty()) scores.push_back(calc_score(finger_scores, ABIS_FINGER_GOST_THRESHOLD));
					if (!iris_scores.empty()) scores.push_back(calc_score(iris_scores, ABIS_IRIS_THRESHOLD));

					float score = 0.f;
					if (!scores.empty()) score = calc_score(scores, ABIS_INTEGRA_THRESHOLD);

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
