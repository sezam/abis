#pragma once

#ifndef ABISREST_H
#define ABISREST_H

#pragma comment(lib, "cpprest_2_10")
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

#include <time.h>

#define TRACE(msg)            wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

// abis bio data types
#define ABIST_BIO_DATA		0x00
#define ABIST_BIO_FACE		0x01
#define ABIST_BIO_FINGER	0x02
#define ABIST_BIO_IRIS		0x03

enum {
	error = 0,
	user,
	info,
	debug,
};

#endif

