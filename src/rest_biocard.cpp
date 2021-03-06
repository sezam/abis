﻿#include "AbisRest.h"
#include "rest_biocard.h"
#include "restutils.h"
#include "dbclient.h"
#include "ebsclient.h"
#include "fplibclient.h"

http_listener register_biocard(uri url)
{
	http_listener listener(url);
	listener.support(methods::GET, biocard_get);
	listener.support(methods::PUT, biocard_put);
	listener.support(methods::DEL, biocard_del);

	listener
		.open()
		.then([&listener]() { BOOST_LOG_TRIVIAL(info) << "starting to listen biocard"; })
		.wait();

	return listener;
}

void biocard_get(http_request request)
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
				auto sp = uri::split_path(request.relative_uri().to_string());
				if (sp.size() != 1) throw runtime_error("GET biocard/{uuid} expected.");

				string_generator gen;
				uuid gid = gen(utility::conversions::to_utf8string(sp[0]));

				db = db_open();

				string s1 = to_string(gid);
				const char* paramValues[1] = { s1.c_str() };

				sql_res = db_exec_param(db, SQL_TMP_IDS_BY_BC_GID, 1, paramValues, 1);
				if (PQresultStatus(sql_res) == PGRES_TUPLES_OK)
				{
					int type_num = PQfnumber(sql_res, "tmp_type");
					int id_num = PQfnumber(sql_res, "tmp_id");

					auto json_out = json::value::array();
					for (int i = 0; i < PQntuples(sql_res); i++)
					{
						auto json_row = json::value::object();

						char* ptr = PQgetvalue(sql_res, i, type_num);
						int tmp = ntohl(*((uint32_t*)ptr));
						json_row[ELEMENT_TYPE] = json::value::number(tmp);

						ptr = PQgetvalue(sql_res, i, id_num);
						tmp = ntohl(*((uint32_t*)ptr));
						json_row[ELEMENT_ID] = json::value::number(tmp);
						json_out[i] = json_row;
					}
					answer[ELEMENT_VALUE] = json_out;
					answer[ELEMENT_RESULT] = json::value::boolean(true);
				}
				else
				{
					BOOST_LOG_TRIVIAL(debug) << "biocard_get: " << PQerrorMessage(db);
					answer[ELEMENT_ERROR] = json::value::string(conversions::to_string_t(PQerrorMessage(db)));
				}
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

void biocard_put(http_request request)
{
	http::status_code sc = status_codes::BadRequest;

	handle_request(
		request,
		[&](json::value const& req_json, json::value& answer)
		{
			PGconn* transact_db = nullptr;
			PGconn* readonly_db = nullptr;
			try
			{
				readonly_db = db_open();

				transact_db = db_open();
				db_tx_begin(transact_db);			

				uuid gid;
				int bc_id = 0;
				int res = 0;

				auto sp = uri::split_path(request.relative_uri().to_string());
				if (sp.size() == 0)
				{
					boost::uuids::random_generator gen;
					gid = gen();

					bc_id = db_add_bc(transact_db, to_string(gid).c_str(), "");
					if (bc_id <= 0) res = -1;
				}
				if (sp.size() > 0)
				{
					string_generator gen;
					gid = gen(utility::conversions::to_utf8string(sp[0]));

					bc_id = db_get_bc_by_gid(transact_db, to_string(gid).c_str());
					if (bc_id <= 0) res = -2;
				}

				int inserted = 0;
				bool step = res >= 0;
				auto json_out = json::value::array();
				auto arr = req_json.at(ELEMENT_VALUE).as_array();
				for (size_t i = 0; i < arr.size(); i++)
				{
					vector<int> ids;
					void* tmp_in = nullptr;
					int tmp_type = ABIS_DATA;
					int tmp_id = 0;
					auto json_row = json::value::object();

					if (step)
					{
						step = arr[i].at(ELEMENT_TYPE).as_integer() != ABIS_DATA;
						if (!step) res = -3;
						if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error template type - " << res;
					}

					if (step)
					{
						step = tmp_from_json(arr[i], tmp_type, tmp_in) > 0;
						if (!step) res = -4;
						if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error extract bio template - " << res;
					}

					if (step && tmp_type == ABIS_FACE_TEMPLATE)
					{
						db_sp_begin(transact_db, "face_template");
						if (step)
						{
							step = db_search_face_tmp(readonly_db, tmp_in, tmp_id) > 0;
							if (!step) res = -101;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error search face template - " << res;
						}

						if (step)
						{
							void* tmp_db = malloc(ABIS_TEMPLATE_SIZE);
							step = tmp_db != nullptr;
							if (!step) res = -102;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error memory allocate 1 - " << res;

							if (step)
							{
								memset(tmp_db, 0, ABIS_TEMPLATE_SIZE);
								step = db_face_tmp_by_id(readonly_db, tmp_id, tmp_db) > 0;
								if (!step) res = -103;
								if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error get face template - " << res;
							}

							if (step)
							{
								float fscore = cmp_face_tmp(tmp_in, tmp_db);
								if (step && fscore >= ABIS_FACE_THRESHOLD)
								{
									char bc_gid[50];
									step = db_get_bc_for_tmp(transact_db, tmp_type, tmp_id, bc_gid) == 0;
									if (!step) res = -104;
									if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: face template already in biocard - " << res;
								}
								if (step && fscore <= ABIS_EQUAL_THRESHOLD)
								{
									tmp_id = db_get_face_seq(readonly_db);
									step = tmp_id > 0;
									if (step) step = db_insert_face_tmp(readonly_db, tmp_in, tmp_id);
									if (!step) res = -105;
									if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error insert face template - " << res;
								}
							}
							if (tmp_db != nullptr) free(tmp_db);
						}

						if (step)
						{
							step = db_add_link(transact_db, ABIS_FACE_TEMPLATE, tmp_id, bc_id) > 0;
							if (!step) res = -106;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error add face template to biocard - " << res;
						}

						if (step) inserted++;
						if (!step) db_sp_rollback(transact_db, "face_template");
					}

					if (step && tmp_type == ABIS_FINGER_GOST_TEMPLATE)
					{
						db_sp_begin(transact_db, "finger_template");
						if (step)
						{
							step = db_search_finger_tmp(readonly_db, tmp_in, tmp_id) > 0;
							if (!step) res = -301;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error search finger template - " << res;
						}

						float score_tmp = 0.f;
						if (step)
						{
							void* tmp_db = malloc(ABIS_TEMPLATE_SIZE);
							step = tmp_db != nullptr;
							if (!step) res = -302;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error memory allocate 2 - " << res;


							if (step)
							{
								memset(tmp_db, 0, ABIS_TEMPLATE_SIZE);
								step = db_finger_tmp_by_id(readonly_db, tmp_id, tmp_db) > 0;
								if (!step) res = -303;
								if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error get finger template - " << res;
							}
							if (step) score_tmp = cmp_finger_tmp(tmp_in, tmp_db);
							if (tmp_db != nullptr)free(tmp_db);
						}

						float score_gost = 0.f;
						bool is_gost = false;
						if (step)
						{
							if (score_tmp < ABIS_EQUAL_THRESHOLD)
							{
								if (step) step = (tmp_id = db_get_finger_seq(readonly_db)) > 0;
								if (!step) res = -304;
								if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error get finger sequence - " << res;

								if (step) step = db_insert_finger_tmp(readonly_db, tmp_in, tmp_id) > 0;
								if (!step) res = -305;
								if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error insert finger template - " << res;

								if (step) step = db_append_finger_gost(readonly_db, ((uchar*)tmp_in) + ABIS_TEMPLATE_SIZE, tmp_id) > 0;
								if (!step) res = -306;
								if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error insert finger template 2 - " << res;
							}
							else
							{
								void* db_gost = malloc(ABIS_FINGER_TMP_GOST_SIZE);
								step = db_gost != nullptr;
								if (!step) res = -307;
								if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error memory allocate 3 - " << res;

								if (step)
								{
									memset(db_gost, 0, ABIS_FINGER_TMP_GOST_SIZE);
									int gost_len = db_gost_tmp_by_id(readonly_db, tmp_id, db_gost);
									is_gost = gost_len == ABIS_FINGER_TMP_GOST_SIZE;
									step = gost_len >= 0;
									if (!step) res = -308;
									if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error get finger template 2 - " << res;
								}

								if (step) score_gost = cmp_fingerprint_gost_template(((uchar*)tmp_in) + ABIS_TEMPLATE_SIZE, db_gost);
								if (db_gost != nullptr) free(db_gost);
							}
						}

						if (step)
						{
							if (is_gost)
							{
								if (score_gost >= ABIS_FINGER_GOST_THRESHOLD)
								{
									char bc_gid[50];
									step = db_get_bc_for_tmp(transact_db, tmp_type, tmp_id, bc_gid) == 0;
									if (!step) res = -309;
									if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: finger template already in biocard - " << res;
								}
								else
								{
									if (!step) res = -310;
									BOOST_LOG_TRIVIAL(warning) << "biocard_put: bad cmp finger template phase 2 - " << res;
									step = false;
								}
							}
							else
							{
								if (step) step = db_append_finger_gost(readonly_db, ((uchar*)tmp_in) + ABIS_TEMPLATE_SIZE, tmp_id) > 0;
								if (!step) res = -311;
								if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error insert finger template 2 - " << res;
							}
						}

						if (step)
						{
							step = db_add_link(transact_db, ABIS_FINGER_GOST_TEMPLATE, tmp_id, bc_id);
							if (step) inserted++;
							if (!step) res = -312;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error insert finger template in biocard - " << res;
						}
						else db_sp_rollback(transact_db, "finger_template");
					}

					if (step && tmp_type == ABIS_IRIS_TEMPLATE)
					{
						db_sp_begin(transact_db, "iris_template");
						if (step)
						{
							step = db_search_iris_tmp(readonly_db, tmp_in, tmp_id) > 0;
							if (!step) res = -501;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error search iris template - " << res;
						}

						if (step)
						{
							void* tmp_db = malloc(ABIS_TEMPLATE_SIZE);
							step = tmp_db != nullptr;
							if (!step) res = -502;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error memory allocate 1 - " << res;

							if (step)
							{
								memset(tmp_db, 0, ABIS_TEMPLATE_SIZE);
								step = db_iris_tmp_by_id(readonly_db, tmp_id, tmp_db) > 0;
								if (!step) res = -503;
								if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error get iris template - " << res;
							}

							if (step)
							{
								float fscore = cmp_iris_tmp(tmp_in, tmp_db);
								if (step && fscore >= ABIS_IRIS_THRESHOLD)
								{
									char bc_gid[50];
									step = db_get_bc_for_tmp(transact_db, tmp_type, tmp_id, bc_gid) == 0;
									if (!step) res = -504;
									if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: iris template already in biocard - " << res;
								}
								if (step && fscore <= ABIS_EQUAL_THRESHOLD)
								{
									tmp_id = db_get_iris_seq(readonly_db);
									step = tmp_id > 0;
									if (step) step = db_insert_iris_tmp(readonly_db, tmp_in, tmp_id);
									if (!step) res = -505;
									if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error insert iris template - " << res;
								}
							}
							if (tmp_db != nullptr) free(tmp_db);
						}

						if (step)
						{
							step = db_add_link(transact_db, ABIS_IRIS_TEMPLATE, tmp_id, bc_id) > 0;
							if (!step) res = -506;
							if (!step) BOOST_LOG_TRIVIAL(debug) << "biocard_put: error add iris template to biocard - " << res;
						}

						if (step) inserted++;
						if (!step) db_sp_rollback(transact_db, "iris_template");
					}

					if (step) json_row[ELEMENT_ID] = json::value::number(tmp_id);
					if (!step) answer[ELEMENT_ERROR] = json::value::number(0x4000 + abs(res));
					json_row[ELEMENT_TYPE] = json::value::number(tmp_type);
					json_row[ELEMENT_RESULT] = json::value::boolean(step);

					json_out[i] = json_row;

					if (tmp_in != nullptr) free(tmp_in);
				}
				//if (inserted > 0)
				if (step)
				{
					db_tx_commit(transact_db);
					answer[ELEMENT_UUID] = json::value::string(conversions::to_string_t(to_string(gid)));
				}
				else db_tx_rollback(transact_db);

				answer[ELEMENT_VALUE] = json_out;
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
			db_close(transact_db);
			db_close(readonly_db);
		});

	request.reply(sc, "");
}

void biocard_del(http_request request)
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
				auto sp = uri::split_path(request.relative_uri().to_string());
				if (sp.size() != 1) throw runtime_error("DELETE biocard/{uuid} expected.");

				string_generator gen;
				uuid gid = gen(utility::conversions::to_utf8string(sp[0]));
				string gid_s = to_string(gid);

				db = db_open();

				if (req_json.is_null())
				{
					int del_link_res = db_del_links(db, gid_s.c_str());
					int del_bc_res = db_del_bc(db, gid_s.c_str());

					answer[ELEMENT_RESULT] = json::value::boolean(del_link_res >= 0 && del_bc_res > 0);
				}
				else
				{
					auto json_out = json::value::array();
					auto arr = req_json.as_array();
					for (size_t i = 0; i < arr.size(); i++)
					{
						int tmp_type = arr[i].at(ELEMENT_TYPE).as_integer();
						int tmp_id = arr[i].at(ELEMENT_ID).as_integer();
						int del_res = db_del_link(db, tmp_type, tmp_id, gid_s.c_str());

						auto json_row = arr[i];
						json_row[ELEMENT_RESULT] = json::value::boolean(del_res > 0);
						json_out[i] = json_row;
					}
					answer[ELEMENT_RESULT] = json::value::boolean(true);
					answer[ELEMENT_VALUE] = json_out;
				}
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
