#pragma once

#ifndef RESTUTILS_H
#define RESTUTILS_H

#include "AbisRest.h"

#define ELEMENT_TYPE	U("type")
#define ELEMENT_VALUE	U("value")

#define ELEMENT_RESULT	U("ok")
#define ELEMENT_ERROR	U("error")

std::wstring s2ws(const std::string& str);
std::string ws2s(const std::wstring& wstr);

void display_json(json::value const & jvalue, std::string const & prefix);
void handle_request(http_request request, function<void(json::value const &, json::value &)> action);

#endif
