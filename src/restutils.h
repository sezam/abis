#pragma once

#ifndef RESTUTILS_H
#define RESTUTILS_H

#include "AbisRest.h"

#define ELEMENT_TYPE	U("type")
#define ELEMENT_VALUE	U("value")

#define ELEMENT_RESULT	U("ok")
#define ELEMENT_ERROR	U("error")

void display_json(json::value const & jvalue, utility::string_t const & prefix);
void handle_request(http_request request, function<void(json::value const &, json::value &)> action);

#endif
