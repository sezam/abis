#pragma once
#pragma execution_character_set("utf-8")

#ifndef ABISREST_H
#define ABISREST_H

//system dependency
#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#define _CRT_SECURE_NO_WARNINGS
#pragma warning( disable : 4996)
#endif

#define _CRT_NONSTDC_NO_DEPRECATE
#define __STDC_WANT_LIB_EXT1__ 1

#define BOOST_NO_ANSI_APIS

#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <limits>
#include <cstdint>
#include <cmath>
using namespace std;

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <chrono>
using namespace std::chrono;

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/beast/core/detail/base64.hpp>
using namespace boost::beast::detail;

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/connect.hpp>
using namespace boost::asio;
using boost::asio::ip::tcp;

#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
using boost::lambda::bind;
using boost::lambda::var;
using boost::lambda::_1;

#include <boost/log/trivial.hpp>
#include "boost/log/utility/setup.hpp"
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/detail/config.hpp>
namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;

#include <boost/interprocess/sync/interprocess_semaphore.hpp>
using namespace boost::interprocess;

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
using namespace boost::uuids;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace utility;

#include <libpq-fe.h>
#ifndef _WIN32
#include <server/postgres_fe.h>
#endif
#include <server/port/pg_bswap.h>

#ifdef _WIN32
#define	STD_TO_UTF(mbstr) to_utf<char>(mbstr, "cp1251")
#else
#define	STD_TO_UTF(mbstr) mbstr
#endif

// abis bio data types
#define ABIS_DATA					0x00
#define ABIS_FACE_IMAGE				0x01
#define ABIS_FINGER_IMAGE			0x02
#define ABIS_FINGER_GOST_IMAGE		0x03
#define ABIS_LIVEFACE_IMAGE			0x04

#define ABIS_FACE_TEMPLATE			0x11
#define ABIS_FINGER_TEMPLATE		0x12
#define ABIS_FINGER_GOST_TEMPLATE	0x13

#define ABIS_INTEGRA_THRESHOLD		0.5f
#define ABIS_FACE_THRESHOLD			0.5f
#define ABIS_FINGER_THRESHOLD		0.42f
#define ABIS_FINGER_GOST_THRESHOLD	0.22f
#define ABIS_EQUAL_THRESHOLD		0.95f

#define ABIS_TEMPLATE_LEN			512 
#define ABIS_TEMPLATE_SIZE			ABIS_TEMPLATE_LEN * sizeof(float)

void load_settings(char* path);
void JSON_EXCEPTION(web::json::value& obj, const string msg);

extern string extract_host;
extern int extract_port_start;
extern int extract_port_count;

extern string logging_path;
extern string logging_level;

extern string postgres_host;
extern string postgres_port;
extern string postgres_db;
extern string postgres_user;
extern string postgres_pwd;

extern int face_parts;
extern string face_vector;
extern string face_index;
extern string face_param;

extern int finger_parts;
extern string finger_vector;
extern string finger_index;
extern string finger_param;

extern float threshold_min;
extern float threshold_max;
extern float ir_threshold;
extern float fish_threshold;
extern float ir_threshold_select;
extern float ensembled_threshold;

extern int finger_min_points;
extern int finger_min_quality;
extern int finger_min_goodpoints;

#endif

