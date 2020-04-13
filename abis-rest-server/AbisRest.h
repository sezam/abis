#pragma once
#pragma execution_character_set("utf-8")

#ifndef ABISREST_H
#define ABISREST_H

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
//system dependency
#define _WIN32_WINNT 0x0601
#endif

#ifdef _DEBUG
#pragma comment(lib, "cpprest_2_10d")
#else
#pragma comment(lib, "cpprest_2_10")
#endif

#define BOOST_NO_ANSI_APIS

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace utility;

#include <iostream>
#include <map>
#include <set>
#include <string>
using namespace std;
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


#ifdef _WIN32
#define	STD_TO_UTF(mbstr) to_utf<char>(mbstr, "cp1251")
#else
#define	STD_TO_UTF(mbstr) mbstr
#endif

#define TRACE(msg)            wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

// abis bio data types
#define ABIS_BIO_DATA		0x00
#define ABIS_BIO_FACE		0x01
#define ABIS_BIO_FINGER		0x02
#define ABIS_BIO_IRIS		0x03

enum {
	log_error = 0,
	log_user,
	log_info,
	log_debug,
};

#endif

