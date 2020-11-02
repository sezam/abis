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
		[&sc](json::value const& req_json, json::value& answer)
		{
			vector<PGconn*> dbs;
			try
			{
				json::array arr = req_json.as_array();
				auto json_out = json::value::array();

				pplx::task<vector<pplx::task<void>>>([&arr, &json_out, &dbs]()
					{
						vector<pplx::task<void>> vv;
						for (size_t i = 0; i < arr.size(); i++)
						{
							auto tt = pplx::task<void>([i, &arr, &json_out, &dbs]()
								{
									BOOST_LOG_TRIVIAL(debug) << "start task " << i;

									PGconn* db = db_open();
									dbs.push_back(db);

									auto json_row = json::value::object();
									vector<int> ids;
									int tmp_type = ABIS_DATA;
									void* tmp_in = nullptr;
									bool step = false;
									bool is_compare = true;

									int search_count = 1;
									if (arr[i].has_field(ELEMENT_COUNT)) search_count = arr[i].at(ELEMENT_COUNT).as_integer();
									if (arr[i].has_field(ELEMENT_COMPARE)) is_compare = arr[i].at(ELEMENT_COMPARE).as_bool();

									int element_type = arr[i].at(ELEMENT_TYPE).as_integer();
									if (element_type != ABIS_DATA)
									{
										step = tmp_from_json(arr[i], tmp_type, tmp_in);
										if (!step) BOOST_LOG_TRIVIAL(debug) << "search_get: error extract bio template";
									}

									if (step)
									{
										if (tmp_type == ABIS_FACE_TEMPLATE)
										{
											step = db_search_face_tmps(db, tmp_in, search_count, ids) > 0;
											if (!step) BOOST_LOG_TRIVIAL(debug) << "search_get: error search face template";
										}
										if (tmp_type == ABIS_FINGER_GOST_TEMPLATE)
										{
											step = db_search_finger_tmps(db, tmp_in, search_count, ids) > 0;
											if (!step) BOOST_LOG_TRIVIAL(debug) << "search_get: error search finger template";
										}
										if (tmp_type == ABIS_IRIS_TEMPLATE)
										{
											step = db_search_iris_tmps(db, tmp_in, search_count, ids) > 0;
											if (!step) BOOST_LOG_TRIVIAL(debug) << "search_get: error search iris template";
										}
									}

									if (step)
									{
										auto json_tmps = json::value::array();
										for (size_t j = 0; j < ids.size(); j++)
										{
											auto json_tmp = json::value::object();
											int tmp_id = ids[j];
											float score = 0;

											if (tmp_type == ABIS_FACE_TEMPLATE && is_compare)
											{
												step = db_tmp_cmp_by_id(db, tmp_type, tmp_in, tmp_id, score) > 0;
											}
											if (tmp_type == ABIS_FINGER_GOST_TEMPLATE && is_compare)
											{
												step = db_tmp_cmp_by_id(db, tmp_type, tmp_in, tmp_id, score) > 0;
											}
											if (tmp_type == ABIS_IRIS_TEMPLATE && is_compare)
											{
												step = db_tmp_cmp_by_id(db, tmp_type, tmp_in, tmp_id, score) > 0;
											}

											if (step)
											{
												char bc_gid[50];
												if (db_get_bc_for_tmp(db, tmp_type, tmp_id, bc_gid) > 0)
												{
													json_tmp[ELEMENT_UUID] = json::value::string(conversions::to_string_t(bc_gid));
													if (is_compare) json_tmp[ELEMENT_VALUE] = json::value::number(score);
												}
											}
											json_tmp[ELEMENT_ID] = json::value::number(tmp_id);
											json_tmp[ELEMENT_RESULT] = json::value::boolean(step);
											json_tmps[j] = json_tmp;
										}

										json_row[ELEMENT_TYPE] = json::value::number(tmp_type);
										json_row[ELEMENT_VALUE] = json_tmps;
									}

									json_out[i] = json_row;
									if (tmp_in != nullptr) free(tmp_in);

									BOOST_LOG_TRIVIAL(debug) << "end task " << i;
								});

							vv.push_back(tt);
						}
						return pplx::task_from_result <vector<pplx::task<void>>>(vv);
					}).then([](pplx::task<vector<pplx::task<void>>> prevTask)
						{
							for (auto v : prevTask.get()) v.wait();
						}).wait();

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

			for (auto db : dbs) db_close(db);
		});
	request.reply(sc, "");
}
