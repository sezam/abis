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

#define BOOST_NO_ANSI_APIS
#define __STDC_WANT_LIB_EXT1__ 1

#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <limits>
using namespace std;

#include <chrono>
using namespace std::chrono;

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/connect.hpp>
using namespace boost::asio;
using boost::asio::ip::tcp;

#include <boost/locale.hpp>
using namespace boost::locale;
using namespace boost::locale::conv;

#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
using boost::lambda::bind;
using boost::lambda::var;
using boost::lambda::_1;

#include <boost/interprocess/sync/interprocess_semaphore.hpp>
using namespace boost::interprocess;

#include <boost/regex.hpp>

#include <boost/gil.hpp>
#include <boost/gil/extension/dynamic_image/any_image.hpp>
#include <boost/gil/extension/io/bmp.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/extension/io/jpeg.hpp>
#include <boost/mp11.hpp>
namespace gil = boost::gil;

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>
using namespace boost::uuids;

//после Boost иначе линкер сругается
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace utility;

#include <libpq-fe.h>
#include <server/catalog/pg_type_d.h>

#ifdef _WIN32
#define	STD_TO_UTF(mbstr) to_utf<char>(mbstr, "cp1251")
#else
#define	STD_TO_UTF(mbstr) mbstr
#endif

#define TRACE(msg)            wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << " (" << k.c_str() << ", " << v.c_str() << ")\n"

// abis bio data types
#define ABIS_DATA				0x00
#define ABIS_FACE_IMAGE			0x01
#define ABIS_FINGER_IMAGE		0x02
#define ABIS_IRIS_IMAGE			0x03
#define ABIS_FACE_TEMPLATE		0x11
#define ABIS_FINGER_TEMPLATE	0x12
#define ABIS_IRIS_TEMPLATE		0x13

#endif

