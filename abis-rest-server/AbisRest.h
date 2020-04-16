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

#ifdef _DEBUG
#pragma comment(lib, "cpprest_2_10d")
#else
#pragma comment(lib, "cpprest_2_10")
#endif
#endif

#define BOOST_NO_ANSI_APIS

#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
using namespace std;

#include <chrono>
using namespace std::chrono;

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace utility;

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

#endif

